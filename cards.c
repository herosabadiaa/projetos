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

void criar_catalogo(CartaCSV catalogo[MAX_CATALOGO]) {
    int cartaCarregando = 0;
    char linha[256];
    FILE *arquivo = fopen("cards.csv", "r"); //Abre o arquivo em leitura
    if (arquivo == NULL) {
        printf("Erro ao abrir\n");
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
    return catalogo
}

void catalogo_textura(CartaCSV catalogo[MAX_CATALOGO],Texture2D texturasCarregadas[MAX_CATALOGO]){
    FOR(i, cartaCarregando) {//Percorre o catalogo pegando os paths e atribuindo as texturas 
        Image img = LoadImage(catalogo[i].caminhoTextura);
        ImageResize(&img, 96, 144);
        texturasCarregadas[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }
    return texturasCarregadas
}


Cabecalho encadeada;

Node* criar_lista(void) {
    encadeada.inicio = (Node*) malloc(sizeof(Node));
    if (encadeada.inicio == NULL) {
        printf("Erro crítico: Falha na alocação de memória para o nó cabeçalho.\n");
        return NULL;
    }
    encadeada.inicio->data = 0;
    encadeada.inicio->next = NULL;
    return encadeada.inicio;
}

bool inserir_inicio(Cabecalho encadeada, int valor) {
    if (encadeada.inicio == NULL) return false;

    Node *novo_no = (Node*) malloc(sizeof(Node));
    if (novo_no == NULL) {
        printf("Erro: Falha na alocação de memória para novo nó.\n");
        return false;
    }

    novo_no->data = valor;
    novo_no->next = encadeada.inicio->next;
    encadeada.inicio->next = novo_no;

    return true;
}


bool inserir_fim(Cabecalho encadeada, int valor) {
    if (encadeada.fim == NULL) return false;

    Node *novo_no = (Node*) malloc(sizeof(Node));
    if (novo_no == NULL) {
        printf("Erro: Falha na alocação de memória para novo nó.\n");
        return false;
    }

    novo_no->data = valor;
    novo_no->next = NULL;

    Node *atual = encadeada.fim;
    while (atual->next != NULL) {
        atual = atual->next;
    }

    atual->next = novo_no;
    return true;
}


Node* buscar(Cabecalho encadeada, int valor) {
    if (encadeada.inicio == NULL) return NULL;

    Node *atual = encadeada.inicio->next;
    while (atual != NULL) {
        if (atual->data == valor) {
            return atual;
        }
        atual = atual->next;
    }
    return NULL;
}


bool remover(Cabecalho encadeada, int valor) {
    if (encadeada.fim == NULL || encadeada.fim == NULL) {
        return false;
    }

    Node *anterior = encadeada.fim;
    Node *atual = encadeada.fim->next;

    while (atual != NULL && atual->data != valor) {
        anterior = atual;
        atual = atual->next;
    }

    if (atual == NULL) {
        return false;
    }

    anterior->next = atual->next;
    free(atual);
    return true;
}


void exibir_lista(Cabecalho encadeada) {
    if (encadeada.inicio == NULL) return;

    Node *atual = encadeada.inicio->next;
    printf("Cabeçalho -> ");
    while (atual != NULL) {
        printf("[%d] -> ", atual->data);
        atual = atual->next;
    }
    printf("NULL\n");
}


void destruir_lista(Cabecalho encadeada) {
    if (cabecalho == NULL) return;

    Node *atual = cabecalho;
    while (atual != NULL) {
        Node *proximo = atual->next;
        free(atual);
        atual = proximo;
    }
}