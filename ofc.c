//----------------------------------------------------------------------------------
// Libraries
//----------------------------------------------------------------------------------
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_DECK 60 //Define o maximo de cartas num deck/cemiterio
#define MAX_HAND 6 //Define o maximo de cartas na mao
#define MAX_POSITION 6//Define o maximo de posicoes

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef struct Carta { //Estruura de uma carta
    Texture2D textura; //Textura da carta
    Vector2 cordenada; //Onde a carta se localiza
    int id; //Id único 
    int hp; //Vida da carta
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


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void){
    //------------------------------------------------------------------------------
    // Window Initialization
    //------------------------------------------------------------------------------
    const int screenWidth = 800; //Define a largura da tela
    const int screenHeight = 450; //Define a altura da tela
    InitWindow(screenWidth, screenHeight, "Dyckson Guloso Adventure"); //Inicializa a janela
    SetTargetFPS(60); //Define o FPS  
    
    //------------------------------------------------------------------------------
    // Deck & Images Initialization
    //------------------------------------------------------------------------------
    Deck deck; //Instancia o Deck
    deck.area = (Rectangle){screenWidth-50, screenHeight-50, 50, 50}; //Define a area que o deck ocupara
    Image imagem1 = LoadImage("images/azul.png"); //Define a imagem 
    Image imagem2 = LoadImage("images/verde.png"); 
    ImageResize(&imagem1, 40, 60); //Modifica o tamanho da imagem
    ImageResize(&imagem2, 40, 60); //
    Texture2D textura = LoadTextureFromImage(imagem1); //Atribui a imagem a uma textura
    Texture2D textura2 = LoadTextureFromImage(imagem2);
    for(int i=0;i<MAX_DECK;i++){ //Percorre as cartas abribuindo uma textura e id
        if(i%2 == 0){
            deck.cartas[i].textura = textura;
        } else{
            deck.cartas[i].textura = textura2;
        }
        deck.cartas[i].id = i+1;
        deck.cartas[i].hp = 2000;
    }
    UnloadImage(imagem1); //Descarrega as imagens da memoria
    UnloadImage(imagem2); 
 
    //------------------------------------------------------------------------------
    // Game Entities Initialization
    //------------------------------------------------------------------------------    
    Posicao mao[6] = {0}; //Instancia a mao
    for(int i=0; i<6; i++){ //Percorre a mao atribuindo uma posicao para cada posicao, e definindo a posicao como vazia
        mao[i].area = (Rectangle){(screenWidth/2-screenWidth/4) + (56.6*i)+45.3, screenHeight/2-screenHeight/4+7.5, 40, 60};
        mao[i].vazia = true;
    }
    int quantPuxada = 0; //Intancia a variavel que conta qnts vezes uma carta foi puxada
    int quantMorta = 0; //Variavel q mostra qnts cartas morreram
    Carta arrasto = {0};
    bool alterMao = false;

    Posicao campos[MAX_POSITION]; //Instancia as posicoes do campo
    for(int i = 0; i<MAX_POSITION; i++){
        campos[i].area.x = i*screenWidth/MAX_POSITION;
        campos[i].area.y = 3*screenHeight/4;
        campos[i].area.width = 80;
        campos[i].area.height =120;
        campos[i].vazia = true;
    }

    Deck cemiterio;
    cemiterio = (Deck){0};
    cemiterio.area = (Rectangle){screenWidth-50, screenHeight-150, 50, 50};

    while(!WindowShouldClose()){
        Vector2 mousePosition = GetMousePosition(); //Pega a posicao do mouse

        if (arrasto.id != 0){ //Altera a posicao de uma carta que esta sendo arrastada
            arrasto.cordenada = (Vector2){mousePosition.x-20, mousePosition.y-30};
        }

        //--------------------------------------------------------------------------
        // Buttons functions
        //--------------------------------------------------------------------------

        //--LMB--
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ //Verifica Click LMB(Left Mouse Button)
            if(quantPuxada < MAX_DECK){ //Nao permite que vc puxe mais do que tem
                if (CheckCollisionPointRec(GetMousePosition(), deck.area)){ //Checa a colisão entre o mouse e deck
                    for(int i=0; i<MAX_HAND; i++){ 
                        if(mao[i].vazia){ //Verifica se uma posicao da mao esta vazia
                            Carta disponivel;
                            int j;
                            for(j = 0; j<MAX_DECK; j++){
                                if(deck.cartas[j].id != 0){
                                    disponivel = deck.cartas[j]; //Ve qual carta esta disponivel, ascendente
                                    break;
                                }
                            }
                            mao[i].carregada = disponivel; //Atribui a carta disponivel a minha mao
                            deck.cartas[j] = (Carta){0}; //A posicao da carta que estava disponivel e zerada
                            mao[i].vazia = false; //A posicao da mao fica cheia
                            quantPuxada += 1;
                            break;
                        }
                    }
                }
            }
            for (int i=0;i<MAX_HAND; i++){ //Verfica a colisao do mouse com as posicoes da mao e se a UI esta ativa
                if (CheckCollisionPointRec(GetMousePosition(), mao[i].area) && alterMao){
                    if (mao[i].vazia == false && arrasto.id == 0){ //Se a colisao for com uma posicao cheia, a posicao e limpa e a uma flag e ativa
                        arrasto = mao[i].carregada;
                        mao[i].carregada = (Carta){0};
                        mao[i].vazia = true;
                    }
                }
            }
            if (arrasto.id != 0){ //Se o arrasto estiver ativo, vai checar a colisao do seu mouse com algum campo
                for(int i = 0; i<MAX_POSITION; i++){
                    if((CheckCollisionPointRec(GetMousePosition(), campos[i].area))){ 
                        if(campos[i].vazia == true){ //Se o campo estiver vazio, atribui a carta do arrasto a ele
                            campos[i].carregada = arrasto;
                            arrasto = (Carta){0}; //Limpa o arrasto, deixando o 0/Falso
                            campos[i].vazia = false; //Preenche a posicao
                            break;
                        }
                    }
                }
            }
        }
        //--RMB--
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            for(int i = 0; i<MAX_POSITION; i++){
                if((CheckCollisionPointRec(GetMousePosition(), campos[i].area) && campos[i].vazia == false)){
                    campos[i].carregada.hp -= 1000;
                }
            }       
        }

        //--Q--
        if(IsKeyPressed(KEY_Q)){
            alterMao = (!alterMao);     
        }

        //Cards Updates
        for (int i=0; i<MAX_POSITION; i++){
            if (campos[i].carregada.hp <= 0 && campos[i].vazia == false){
                for(int j = 0; j<MAX_DECK; j++){
                    if(cemiterio.cartas[j].id == 0){
                        cemiterio.cartas[j] = campos[i].carregada;
                        campos[i].carregada = (Carta){0};
                        campos[i].vazia = true;
                        quantMorta += 1;
                        break;
                    }
                }
                break;
            }
        }
                          
        //--------------------------------------------------------------------------
        // 2. Rendering
        //--------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(WHITE);

            DrawRectangle(deck.area.x, deck.area.y, deck.area.width, deck.area.height, GREEN); //Desenha a area do deck
            DrawRectangle(cemiterio.area.x, cemiterio.area.y, cemiterio.area.width, cemiterio.area.height, BLACK); //Desenha a area do cemiterio

            if(alterMao){
                DrawRectangle(screenWidth/2-screenWidth/4, screenHeight/2-screenHeight/4, screenWidth/2+10, screenHeight/6, YELLOW); //Desenha a mao
                    for(int i=0; i<6; i++){
                        DrawRectangle(mao[i].area.x, mao[i].area.y, 40, 60, YELLOW); //Desenha as areas
                        DrawRectangleLines(mao[i].area.x, mao[i].area.y, 40, 60, BLACK); //Desenha um contorno
                        if(mao[i].vazia == false){ // Se a posicao tiver cheia, desenha a carta que a ocupa
                            DrawTexture(mao[i].carregada.textura, mao[i].area.x, mao[i].area.y, WHITE);
                        }
                    }
                char deckText[64]; 
                char cemiterioText[64];
                DrawText(deckText, 6.5*screenWidth/8, 20, 20, BLACK);
                DrawText(cemiterioText, 6*screenWidth/8, 40, 20, BLACK);
                snprintf(cemiterioText, sizeof(cemiterioText), "CEMITERIO: %d/%d", quantMorta, MAX_DECK);
                snprintf(deckText, sizeof(deckText), "DECK: %d/%d", MAX_DECK-quantPuxada, MAX_DECK);
            }   

            for(int i=0;i<MAX_POSITION;i++){
                DrawRectangle(campos[i].area.x, campos[i].area.y, campos[i].area.width, campos[i].area.height, GRAY); // Desenha as posicoes do campo
                if(campos[i].vazia == false){ //Desenha as cartas caso haja alguma
                    DrawTexturePro(campos[i].carregada.textura, (Rectangle){ 0, 0, campos[i].carregada.textura.width, campos[i].carregada.textura.height }, (Rectangle){ campos[i].area.x, campos[i].area.y, 80.0f, 120.0f }, (Vector2){ 0, 0 }, 0.0f, WHITE);
                }
            }

            if (arrasto.id != 0){ //Desenha a carta sendo arrastada
                DrawTexture(arrasto.textura, arrasto.cordenada.x, arrasto.cordenada.y, WHITE);
            }

        EndDrawing();
    }

    UnloadTexture(textura);
    UnloadTexture(textura2);
    CloseWindow();
    return 0;
}