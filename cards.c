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



Deck carregar_deck(Texture2D texturasCarregadas[MAX_CATALOGO], CartaCSV catalogo[MAX_CATALOGO]){
    Deck deck;
    FOR(i,MAX_DECK) {
        int escolhido = rand() % MAX_CATALOGO; //Seleciona cartas aleatorias do catalogo e atribui ao deck
        strncpy(deck.cartas[i].nome, catalogo[escolhido].nome, sizeof(deck.cartas[i].nome) - 1);
        deck.cartas[i].hp = catalogo[escolhido].hp;
        deck.cartas[i].atk = catalogo[escolhido].atk;
        deck.cartas[i].cost = catalogo[escolhido].mana;
        deck.cartas[i].textura = texturasCarregadas[escolhido];
        deck.cartas[i].id = i + 1;
        deck.cartas[i].lock = true; //Impede jogar a carta no mesmo turno da compra
        deck.cartas[i].shield = false;
    }
    return deck;
}

void criar_posicao(Posicao campos[],int screenWidth, int screenHeight, float y){
    FOR(i, MAX_POSITION){
        campos[i].area.x = i*screenWidth/MAX_POSITION;
        campos[i].area.y = y*screenHeight/4;
        campos[i].area.width = 192;
        campos[i].area.height =288;
        campos[i].vazia = true;
    }
}

bool cemiterio_inserir(CemiterioEncad *cemiterio, Carta carta) {
    Node *novo = (Node *)malloc(sizeof(Node));
    if (novo == NULL) {
        return false;
    }
    novo->carta = carta;
    novo->next = cemiterio->inicio;
    cemiterio->inicio = novo;
    cemiterio->tamanho++;
    return true;
}

void cemiterio_remover(CemiterioEncad *cemiterio) {
    Node *atual = cemiterio->inicio;
    while (atual != NULL) {
        Node *proximo = atual->next;
        free(atual);
        atual = proximo;
    }
    cemiterio->inicio = NULL;
    cemiterio->tamanho = 0;
}