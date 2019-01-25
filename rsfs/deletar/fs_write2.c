int fs_write(char *buffer, int size, int file) {

	printf("buffer: %s\n", buffer);
	printf("size: %d\n", size);
	printf("file: %d\n", file);
	
	// Verifica se o arquivo está aberto
	if (arq[file].estado == FECHADO){
		printf("Arquivo não está aberto!\n");
		return -1;
	}

	// dir[file].size += size;
	int bytes_escritos = 0;

	//Procurar pelo último bloco do arquivo
	int i = arq[file].first_block;
	while (fat[i] != ULTIMO_AGRUPAMENTO){	
		i = fat[i];
	}
	int ultimo_agrupamento = i;

    
	int tamanho_ultimo = arq[file].tamanho % CLUSTERSIZE;
	printf("tamanho_ultimo: %d\n", tamanho_ultimo);
	while (tamanho_ultimo >= CLUSTERSIZE) {

		// Calcula o tamanho efetivo (tamanho que podemos escrever)
		int size_efetivo = CLUSTERSIZE - tamanho_ultimo;
		printf("size_efetivo: %d\n", size_efetivo);

		// Escreve o que cabe no conteúdo do arquivo
		int k = 0;
		for(int j = tamanho_ultimo; j < size_efetivo; j++, k++) {
			if(j < size) {
				arq[file].conteudo[j] = buffer[k];
				bytes_escritos++;
			}
		}

		// Salva no disco
		if(salvar_disk(file, ultimo_agrupamento) == -1){
			printf("Erro ao escrever no disco!\n");
			return -1;
		}

		memset(arq[file].conteudo, 0, CLUSTERSIZE+1);

		if (tamanho_ultimo == CLUSTERSIZE) {
			printf("Vai procurar um novo agrupamento\n");
			// Procura por um agrupamento livre na FAT
			i = 0;
			while (fat[i] != AGRUPAMENTO_LIVRE && i < TAMANHO_FAT) {
				i++;
			}
			fat[i] = ULTIMO_AGRUPAMENTO;
			fat[ultimo_agrupamento] = i;
			ultimo_agrupamento = i;

			arq[file].n_blocos++;
		}

	}

	// strncat(arq[file].conteudo, buffer, size);
	int j = arq[file].tamanho;
	for(int k = 0; k < size; k++, j++) {
		arq[file].conteudo[j] = buffer[k];
		bytes_escritos++;
	}

	dir[file].size += bytes_escritos;
	arq[file].tamanho += bytes_escritos;

	printf("conteudo: %s\n", arq[file].conteudo);
	
	return size;


	/* Salva a FAT e o diretório */
	if (salvar_fat() == 0)
		return -1;

	if (salvar_dir() == 0)
		return -1;

}