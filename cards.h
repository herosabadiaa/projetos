//----------------------------------------------------------------------------------
// Libraries & etc.
//----------------------------------------------------------------------------------
#ifndef CARDS_H
#define CARDS_H

#include <stdio.h>
#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_DECK 60 //Define o maximo de cartas num deck/cemiterio
#define MAX_POSITION 6//Define o maximo de posicoes e cartas na mao
#define MAX_CATALOGO 131
#define MAX_MANA 200

#define FOR(i,n) for(int i=0; i<(n);i++)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef struct Carta { //Estruura de uma carta
    Texture2D textura; //Textura da carta
    Vector2 cordenada; //Onde a carta se localiza
    int id; //Id único 
    int hp; //Vida da carta
    int atk; //Ataque da carta
    int cost;
    bool lock;
    bool shield; 
    char nome[64];
} Carta;

typedef struct Posicao { //Estrutura de posições no campo
    Rectangle area; //Local que ocupa da tela
    Carta carregada; //Carta que esta tribuida
    bool vazia; //Posicao vazia?
} Posicao;

typedef struct Deck { //Estrutura do deck
    Rectangle area; //Local que ocupa da tela
    Carta cartas[MAX_DECK]; //Lista de cartas
} Deck;

typedef enum TurnState{ //Estrutura de estados
    TURNO_JOGADOR, //Estado do meu turno
    TURNO_INIMIGO //Estado do turno inimigo
} TurnState;

typedef enum GameState{
    MENU,
    JOGANDO
} GameState;

typedef struct CartaCSV{ //Estrutura para catalogo
    char nome[64];
    int hp;
    int atk;
    int mana;
    char caminhoTextura[128];
} CartaCSV;

typedef struct Node {
    Carta carta;
    struct Node *next;
} Node;

typedef struct ListaCemiterio { 
    Node *inicio;
    int tamanho;
} ListaCemiterio;
//----------------------------------------------------------------------------------
// Functions Headers
//----------------------------------------------------------------------------------

int criar_catalogo(CartaCSV catalogo[MAX_CATALOGO]);

void catalogo_textura(CartaCSV catalogo[MAX_CATALOGO], Texture2D texturasCarregadas[MAX_CATALOGO], int cartaCarregando);

bool cemiterio_inserir(ListaCemiterio *lista, Carta carta);

void cemiterio_remover(ListaCemiterio *lista);

#endif /* CARDS_H */