//----------------------------------------------------------------------------------
// Libraries & etc.
//----------------------------------------------------------------------------------
#include "cards.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

//----------------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------------

int criar_catalogo(CartaCSV catalogo[MAX_CATALOGO]) {
    int cartaCarregando = 0;
    char linha[256];
    FILE *arquivo = fopen("cards.csv", "r"); //Abre o arquivo em leitura
    if (arquivo == NULL) {
        printf("Erro ao abrir\n");
        return 0;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) { //Ler linha por linha ate a ultima
        linha[strcspn(linha, "\r\n")] = '\0'; //remove quebras de linhas
        char *pedaco = strtok(linha, ",");//pega o as partes divididas pela ,
        strncpy(catalogo[cartaCarregando].nome, pedaco, sizeof(catalogo[cartaCarregando].nome) - 1); // atribui a informacao a uma carta do catalogo
        pedaco = strtok(NULL, ",");
        catalogo[cartaCarregando].hp = atoi(pedaco);
        pedaco = strtok(NULL, ",");
        catalogo[cartaCarregando].atk = atoi(pedaco);
        pedaco = strtok(NULL, ",");
        catalogo[cartaCarregando].mana = atoi(pedaco);
        pedaco = strtok(NULL, ",");
        strncpy(catalogo[cartaCarregando].caminhoTextura, pedaco, sizeof(catalogo[cartaCarregando].caminhoTextura) - 1);
        cartaCarregando++;
    }
    fclose(arquivo);
    return cartaCarregando; //devolve quantas cartas foram carregadas
}

void catalogo_textura(CartaCSV catalogo[MAX_CATALOGO], Texture2D texturasCarregadas[MAX_CATALOGO], int cartaCarregando) {
    FOR(i, cartaCarregando) {
        Image img = LoadImage(catalogo[i].caminhoTextura);
        texturasCarregadas[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }
}