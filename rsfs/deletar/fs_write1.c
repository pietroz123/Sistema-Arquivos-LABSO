int fs_write(char *buffer, int size, int file) {


	printf("buffer: %s\n", buffer);
	printf("size: %d\n", size);
	printf("file: %d\n", file);
	
	
	// Verifica se o arquivo está aberto
	if (arq[file].estado == FECHADO){
		printf("Arquivo não está aberto!\n");
		return -1;
	}

	dir[file].size += size;

	//Procurar pelo último bloco do arquivo
	int i = arq[file].first_block;
	while (fat[i] != ULTIMO_AGRUPAMENTO){	
		i = fat[i];
	}
	int ultimo_agrupamento = i;
	// printf("ultimo agrupamento: %d\n", ultimo_agrupamento);
	
	
	int tamanho_ultimo = arq[file].tamanho % CLUSTERSIZE;
	int bytes = 0;
	while (tamanho_ultimo <= CLUSTERSIZE && size > 0) {



		char c = buffer[bytes];
		bytes++;

		/* Concatena byte a byte no conteudo do arquivo */
        int len = strlen(arq[file].conteudo);
        arq[file].conteudo[len] = c;
        arq[file].conteudo[len+1] = '\0';


		/* Salva no disco */
		if(salvar_disk(file, ultimo_agrupamento) == -1){
			printf("Erro ao escrever no disco!\n");
			return -1;
		}

		/* Se for ultrapassar 4096 bytes, procura um novo agrupamento na FAT */
		if (tamanho_ultimo == CLUSTERSIZE) {
			printf("Vai procurar um novo agrupamento\n");
			/* Procura por um agrupamento livre na FAT */
			i = 33;	// Primeiro agrupamento da FAT
			while (fat[i] != AGRUPAMENTO_LIVRE && i < TAMANHO_FAT) {
				i++;
			}
			fat[i] = ULTIMO_AGRUPAMENTO;
			fat[ultimo_agrupamento] = i;
			ultimo_agrupamento = i;

			arq[file].n_blocos++;
		}

		

		tamanho_ultimo++;	// Aumenta a quantidade de bytes no ultimo agrupamento
		size--;				// Diminui o numero de bytes lidos

	}
	
	// printf("conteudo: %s\n", arq[file].conteudo);
	// printf("tamanho_ultimo: %d\n", tamanho_ultimo);
	// printf("size: %d\n", size);


	/* Salva a FAT e o diretório */
	if (salvar_fat() == 0)
		return -1;

	if (salvar_dir() == 0)
		return -1;
	
	
	
	return size;

}