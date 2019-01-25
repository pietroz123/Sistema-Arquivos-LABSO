int fs_read(char *buffer, int size, int file)
{

	printf("buffer: %s\n", buffer);
	printf("size: %d\n", size);
	printf("file: %d\n", file);

	// Verifica se o arquivo está aberto
	if (arq[file].estado == ABERTO)
	{

		// Limpa o buffer
		memset(buffer, 0, size);

		int bloco_atual = arq[file].first_block;
		int byteslidos = 0;
		int i;

		for (i = 0; i < size; i++)
		{
			if (arq[file].indice_relativo + size > CLUSTERSIZE)
			{
				bloco_atual = fat[bloco_atual];
				if (ler_disk(file, bloco_atual) == -1)
				{
					printf("Erro na leitura\n");
					return -1;
				}
				arq[file].indice_relativo = 0;
			}

			if (arq[file].tamanho == dir[file].size)
			{
				return byteslidos;
			}

			buffer[i] = arq[file].conteudo[arq[file].indice_relativo];
			arq[file].indice_relativo++;
			arq[file].tamanho++;
			byteslidos++;
		}
		return size;
	}
	printf("Arquivo nao se encontra em modo de leitura\n");

	return -1;
}