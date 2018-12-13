/*
 * RSFS - Really Simple File System
 *
 * Copyright © 2010 Gustavo Maciel Dias Vieira
 * Copyright © 2010 Rodrigo Rocco Barbieri
 *
 * This file is part of RSFS.
 *
 * RSFS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * 	Membros do grupo:
 * 
 * 		Bianca Gomes Rodrigues			743512
 * 		Pietro Zuntini Bonfim			743588
 * 		Pedro de Souza Vieira Coelho	743585
 * 		Vinícius Brandão Crepschi		743601
 */


#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define CLUSTERSIZE 4096
#define TAMANHO_FAT 65536

unsigned short fat[TAMANHO_FAT];			/* FAT */


typedef struct {
	char used;						/* booleano que indica se a entrada está em uso */
	char name[25];					/* nome do arquivo */
	unsigned short first_block;		/* primeiro agrupamento do arquivo */
	int size;						/* tamanho do arquivo em bytes */
} dir_entry; // 32 bytes

dir_entry dir[128];					/* diretório */


int flagFormatado = 0;


typedef struct {
	int estado;
	unsigned short block;
	char conteudo[CLUSTERSIZE+1];
	int tamanho_atual;
} arquivo;

arquivo arq[128];


/* Funcoes de auxílio */
int salvar_fat() {

	char *buffer = (char*) fat;
	int i;

	/* Salva a FAT no buffer */
	for (i = 0; i < 32*8; i++) {
		if (!bl_write(i, buffer + i*512)) {
			printf("Erro: Falha ao escrever no disco!\n");
			return 0;
		}
	}

	return 1;
}

int salvar_dir () {

	char *buffer = (char*) dir;
	int i;

	/* Salva o diretório */
	int j = 0;
	for (i = 32*8; i < 33*8; i++, j++) {
		if (!bl_write(i, buffer + j*512)) {
			printf("Erro: Falha ao escrever no disco!\n");
			return 0;
		}
	}

	return 1;
}

int salvar_disk( int posicao) {

	for(int i = 0; i < 8; i++){
		if(bl_write(arq[posicao].block * 8 + i, arq[posicao].conteudo[i*512]) == 0){
			return -1;
		}
	}
	return 1;

}

int recuperar_fat()	{

	char *buffer = (char*) fat;
	int i;

	/* Salva o FAT no buffer */
	for (i = 0; i < 32*8; i++) {
		if (!bl_read(i, buffer + i*512)) {
			return 0;
		}
	}

	return 1;
}

int recuperar_dir() {

	char *buffer = (char*) dir;
	int i;

	/* Salva o diretório */
	int j = 0;
	for (i = 32*8; i < 33*8; i++, j++) {
		if (!bl_read(i, buffer + j*512)) {
			return 0;
		}
	}

	return 1;
}

// Retorna a posicao do arquivo com nome file_name no diretorio
int buscar_arquivo(char *file_name) {

	int posicao;
	for(posicao = 0; posicao < 128; posicao++){
		if(strcmp(dir[posicao].name, file_name) == 0){
			break;
		}
	}
	
	return posicao;

}

/* carrega a fat e o diretório */
int fs_init() {


	/* Recupera a FAT e o diretório */
	if (!recuperar_fat()) {
		printf("Erro: Falha ao ler do disco!\n");
		return 0;
	}

	if (!recuperar_dir()) {
		printf("Erro: Falha ao ler do disco!\n");
		return 0;
	}


	int i;
	for (i = 0; i < 32; i++) {
		if (fat[i] != 3) {
			flagFormatado = 0;
		}
	}

	return 1;
}

/* cria novas estruturas na memória e as escreve no disco */
int fs_format() {

	
	int i;

	/* Formata a FAT */
	for (i = 0; i < 32; i++) {
		fat[i] = 3;
	}
	fat[i] = 4;
	i++;
	for (;i < TAMANHO_FAT; i++) {
		fat[i] = 1;
	}

	/* Formata o diretório */
	for (i = 0; i < 128; i++) {
		dir[i].used = 0;
	}

	/* Salva a FAT e o diretório */
	int resFAT = salvar_fat();
	if (resFAT == 0)
		return 0;


	int resDIR = salvar_dir();
	if (resDIR == 0)
		return 0;


	flagFormatado = 1;
	return 1;
}

/* pega a fat e varre todos os campos marcados como 1 (livres) */
int fs_free() {


	int espaco_livre = 0;

	int i = 33;
	while (i < bl_size()/8) {
		if (fat[i] == 1)
			espaco_livre++;
		i++;
	}

	espaco_livre *= CLUSTERSIZE;

	return espaco_livre;
}

/* pega o diretorio, varre as 128 entradas e concatena no buffer */
/* Formato: NOME\t\tTAMANHO\n */ 
int fs_list(char *buffer, int size) {
	
	// Zera o buffer
	// memset(buffer, 0, size);
	buffer[0] = '\0';
	char tamanho[100];

	int i = 0;
	while (i < 128) {
		if (dir[i].used == 1) {
			strcat(buffer, dir[i].name);
			strcat(buffer, "\t\t");
			sprintf(tamanho, "%d", dir[i].size);
			strcat(buffer, tamanho);
			strcat(buffer, "\n");
		}
		i++;
	}
	return 1;
}

int fs_create(char *file_name) {

	int i = 0;

	/* Utilizamos uma flag verificar se há ou não espaço livre no diretório e, caso haja, armazenar esta posição */
	int posicao = -1;


	/* Verifica se o arquivo já existe */
	while (i < 128) {
		if (strcmp(file_name, dir[i].name) == 0) {
			if (dir[i].used == 1) {
				printf("Erro: Arquivo '%s' ja existente!\n", file_name);
				return 0;
			}
		}
		if (posicao == -1 && dir[i].used == 0)
			posicao = i;
		i++;
	}

	/* Verifica se o diretório está cheio */
	if (posicao == -1) {
		printf("Erro: Diretorio cheio!\n");
		return 0;
	}


	// Insere o arquivo no diretório com nome file_name e tamanho 0
	dir[posicao].used = 1;
	strcpy(dir[posicao].name, file_name);
	dir[posicao].size = 0;


	/* Atualiza a FAT */
	i = 33;
	while (i < bl_size()/8) {
		if (fat[i] == 1) {
			fat[i] = 2;
			dir[posicao].first_block = i; // Marca no diretório o primeiro bloco do arquivo
			break;
		}
		
		i++;
	}

	/* Salva a FAT e o diretório */
	int resFAT = salvar_fat();
	if (resFAT == 0)
		return 0;

	int resDIR = salvar_dir();
	if (resDIR == 0)
		return 0;


	return 1;
}

int fs_remove(char *file_name) {

	int i = 0;
	int achou = 0;

	/* Procura o arquivo no diretório */
	while(i < 128) {
		if (strcmp(dir[i].name, file_name) == 0)
			if (dir[i].used == 1) {
				achou = 1;
				break;
			}
		i++;
	}

	/* Se não achou, retorna erro */
	if (achou == 0) {
		printf("Erro: Arquivo inexistente!\n");
		return 0;
	}

	int posicao = i;

	/* Zera no diretório as informações do arquiv */
	dir[posicao].used = 0;
	memset(dir[posicao].name, 0, 25);


	/* Marca todos blocos daquele arquivo como Agrupamento Livre */
	i = dir[posicao].first_block;
	while (fat[i] != 2) {
		posicao = fat[i];
		fat[i] = 1;
		i = posicao;
	}
	fat[i] = 1;	// Zera a última posição


	/* Salva a FAT e o diretório */
	int resFAT = salvar_fat();
	if (resFAT == 0)
		return 0;

	int resDIR = salvar_dir();
	if (resDIR == 0)
		return 0;


	return 1;
}

/* Abre um arquivo pra leitura (FS_R) ou escrita (FS_W) */
/* leitura: cabeça no byte 0 e vai lendo até o último bloco */
/* escrita: zera o arquivo, cabeça no byte 0 e vai até o final */
int fs_open(char *file_name, int mode) {
	//printf("Função não implementada: fs_open\n");

	int i;
	for(i = 0; i < 128; i++){
		if(strcmp(dir[i].name, file_name) == 0){
			break;
		}
	}

	if (i == 128)
		return -1;

	if(mode == FS_R){
		arq[i].block = dir[i].first_block;
		//Estado Aberto
		arq[i].estado = 1;
		arq[i].tamanho_atual = 0;
		return i;
	}
	else if(mode == FS_W){
		if(fs_remove(file_name) == 0){
			printf("Não foi possível sobrescrever o arquivo!\n");
			return -1;
		}
		if(fs_create(file_name) == 0){
			printf("Não foi possível sobrescrever o arquivo!\n");
			return -1;
		}

		arq[i].block = dir[i].first_block;
		//Estado Aberto
		arq[i].estado = 1;
		arq[i].tamanho_atual = 0;
		return i;

	}

	printf("Modo inválido de operação!\n");
	return -1;
}

/* recebe o inteiro que identifica o arquivo aberto */
/* libera o que alocamos no open */
int fs_close(int file) {
	printf("Função não implementada: fs_close\n");
	return 0;
}

/* carrega do buffer para a memória */
/* bl_write sempre buffer de 512 bytes, aqui o buffer é de quanto quisermos */
/* retorna a quantidade de bytes escritos */
int fs_write(char *buffer, int size, int file) {
	
	dir[file].size += size;
	
	// Verifica se o arquivo está aberto
	if (arq[file].estado == 0){
		printf("Arquivo não está aberto!\n");
		return -1;
	}
	else{
		if(arq[file].tamanho_atual + size < CLUSTERSIZE){
			strcat(arq[file].conteudo, buffer);
			arq[file].tamanho_atual += size;
			
			memset(buffer, 0, size);
		}
		else{
			strncat(arq[file].conteudo, buffer, CLUSTERSIZE - arq[file].tamanho_atual);
			memset(buffer, 0, size);

			if(salvar_disk(file) == -1){
				printf("Erro ao escrever no disco!\n");
				return -1;
			}
			memset(arq[file].conteudo, 0, CLUSTERSIZE+1);

			strcat(arq[file].conteudo, buffer);

		}


	}

	/* Salva a FAT e o diretório */
	int resFAT = salvar_fat();
	if (resFAT == 0)
		return -1;


	int resDIR = salvar_dir();
	if (resDIR == 0)
		return -1;

}

/* carrega da memória para o buffer */
/* detalhe: precisamos verificar se o tamanho que queremos ler cabe no tamanho do arquivo */
/* retorna quantidade de bytes EFETIVAMENTE lidos */
int fs_read(char *buffer, int size, int file) {
	printf("Função não implementada: fs_read\n");
	return -1;
}