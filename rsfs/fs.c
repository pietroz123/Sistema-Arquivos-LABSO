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

#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define CLUSTERSIZE 4096

unsigned short fat[65536];			/* FAT */

typedef struct {
	char used;						/* booleano que indica se a entrada está em uso */
	char name[25];					/* nome do arquivo */
	unsigned short first_block;		/* primeiro agrupamento do arquivo */
	int size;						/* tamanho do arquivo em bytes */
} dir_entry; // 32 bytes

dir_entry dir[128];					/* diretório */

/* carrega a fat e o diretório */
int fs_init() {
	printf("Função não implementada: fs_init\n");
	return 1;
}

/* cria novas estruturas na memória e as escreve no disco */
int fs_format() {
	
	int i;
	char *buffer;

	/* Formata a FAT */
	for (i = 0; i < 32; i++) {
		fat[i] = 3;
	}
	fat[i] = 4;
	i++;
	for (;i < 65536; i++) {
		fat[i] = 1;
	}

	/* Formata o diretório */
	for (i = 0; i < 128; i++) {
		dir[i].used = 0;
	}

	buffer = (char*) fat;

	/* Salva o FAT no buffer */
	for (i = 0; i < 32*8; i++) {
		if (!bl_write(i, buffer + i*512)) {
			printf("Erro\n"); //todo
			return 0;
		}
	}

	buffer = (char*) dir;

	/* Salva o diretório */
	int j =0;
	for (i = 32*8; i < 33*8; i++, j++) {
		if (!bl_write(i, buffer + j*512)) {
			printf("Erro\n");
			return 0;
		}
	}


	// printf("Função não implementada: fs_format\n");
	return 1;
}

/* pega a fat e varre todos os campos marcados como 1 (livres) */
int fs_free() {

	int espaco_livre = 0;

	int i = 33;
	while (i < 65536) {
		if (fat[i] == 1)
			espaco_livre++;
	}

	espaco_livre *= CLUSTERSIZE;

	return espaco_livre;
}

/* pega o diretorio, varre as 128 entradas e concatena no buffer */
int fs_list(char *buffer, int size) {
	printf("Função não implementada: fs_list\n");
	return 0;
}

int fs_create(char *file_name) {
	printf("Função não implementada: fs_create\n");
	return 0;
}

int fs_remove(char *file_name) {
	printf("Função não implementada: fs_remove\n");
	return 0;
}

int fs_open(char *file_name, int mode) {
	printf("Função não implementada: fs_open\n");
	return -1;
}

int fs_close(int file) {
	printf("Função não implementada: fs_close\n");
	return 0;
}

int fs_write(char *buffer, int size, int file) {
	printf("Função não implementada: fs_write\n");
	return -1;
}

int fs_read(char *buffer, int size, int file) {
	printf("Função não implementada: fs_read\n");
	return -1;
}
