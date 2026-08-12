//----------------------------------------------------------------------------------
// Libraries
//----------------------------------------------------------------------------------
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_POSITION 5 
#define MAX_DECK 60
#define MAX_CEMITERIO 60

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Cartas {
    Texture2D textura;
    Vector2 posicao;
    int id;
    int hp;
} Cartas;

typedef struct Posicoes {
    Rectangle posicao;
    Cartas carregada;
    bool vazia;
} Posicoes;

typedef struct Deck {
    Rectangle deck_area;
    Cartas deck[MAX_DECK];
} Deck;

typedef struct Cemiterio {
    Cartas cemiterio[MAX_CEMITERIO];
    Rectangle cemit_area;
} Cemiterio;

typedef enum Estado {
    MENU,
    SEU_TURNO,
    OP_TURNO
} Estadp;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    //------------------------------------------------------------------------------
    // Window Initialization
    //------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "CardGameTestFuctions");
    SetTargetFPS(60);  
    
    //------------------------------------------------------------------------------
    // Game Entities Initialization
    //------------------------------------------------------------------------------
    //Instancia o deck
    Deck deck;
    deck.deck_area = (Rectangle){screenWidth-50, screenHeight-50, 50, 50};

    //Instancia as cartasazul
    Image imagem = LoadImage("images/azul.png");
    ImageResize(&imagem, 40, 40);
    Texture2D textura = LoadTextureFromImage(imagem);
    for(int i=0;i<MAX_DECK;i++){
        deck.deck[i].textura = textura;
        deck.deck[i].hp = 2000;
        deck.deck[i].id = i+1;
    }
    UnloadImage(imagem); 

    //Instancia o cemiterio
    Cemiterio cemiterio;
    cemiterio.cemit_area = (Rectangle){screenWidth-50, screenHeight-150, 50, 50};

    //Instancia o estado de ação e controle do mouse
    Vector2 mousePosition = {-100, -100};
    Rectangle areaRec = {-100, -100};
    bool pull = false;
    int carta_num = 0;
    int cemit_num = 0;
    bool vis_cemit = false;
    bool vis_deck = false;
    bool gui_cemit = false;
    bool close = false;
    int id_close;
    int fator_elevar = 0;

    // Cria e define a posição das áreas do campo
    Posicoes areas[MAX_POSITION]; 
    for(int i = 0; i<MAX_POSITION; i++){
        areas[i].posicao.x = i*screenWidth/MAX_POSITION;
        areas[i].posicao.y = 3*screenHeight/4;
        areas[i].posicao.width = 80;
        areas[i].posicao.height =120;
        areas[i].vazia = true;
    }

    //------------------------------------------------------------------------------
    // Main Game Loop
    //------------------------------------------------------------------------------
    while(!WindowShouldClose()){
        // 1. Update / Input Handling
        //--------------------------------------------------------------------------

        //Future trail system
        mousePosition = GetMousePosition();
        areaRec = (Rectangle){mousePosition.x-0.25, mousePosition.y-0.25, 0.5,0.5};

        if (pull){
            deck.deck[carta_num].posicao = (Vector2){mousePosition.x-20, mousePosition.y-20};
        }

        if (close){
            if(IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)){
                close = false;
            }
        }

        // Clique com botão esquerdo
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            if(carta_num < MAX_DECK){
                if (CheckCollisionPointRec(GetMousePosition(), deck.deck_area)){
                    pull = true;
                }
            }
            if (pull){
                for(int i = 0; i<MAX_POSITION; i++){
                    if((CheckCollisionPointRec(GetMousePosition(), areas[i].posicao))){
                        if(areas[i].vazia == true){
                            pull = false;
                            areas[i].carregada = deck.deck[carta_num];
                            areas[i].vazia = false;
                            carta_num += 1;
                        }
                    }
                }
            }
        }

        // Clique com botão direito
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            for(int i = 0; i<MAX_POSITION; i++){
                if((CheckCollisionPointRec(GetMousePosition(), areas[i].posicao))){
                    areas[i].carregada.hp -= 1000;
                    if(areas[i].carregada.hp <= 0 && areas[i].carregada.id != 0){
                        cemiterio.cemiterio[cemit_num] = areas[i].carregada;
                        areas[i].vazia = true;
                        areas[i].carregada = (Cartas){0};
                        cemit_num += 1;
                        close = false;
                    }
                }
            }
        }

        //Clique com o scrool
        if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)){
            for(int i = 0; i<MAX_POSITION; i++){
                if(areas[i].vazia == false){
                    if((CheckCollisionPointRec(GetMousePosition(), areas[i].posicao))){
                        close = true;
                        id_close = i;
                    }
                }
            }
        }

        if (IsKeyPressed(KEY_T)){
            gui_cemit = !gui_cemit;
            close = false;
        }

        // Hovering
        vis_cemit = false;
        vis_deck = false;
        if((CheckCollisionPointRec(GetMousePosition(), cemiterio.cemit_area))){
            vis_cemit = true;
        } else if((CheckCollisionPointRec(GetMousePosition(), deck.deck_area))){
            vis_deck = true;
        }

        // 2. Rendering
        //--------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(WHITE);

            // Interface Deck
            if(vis_deck){
                char deckText[64];
                snprintf(deckText, sizeof(deckText), "DECK: %d/%d", MAX_DECK-carta_num, MAX_DECK);
                DrawText(deckText, 6.5*screenWidth/8, 20, 20, BLACK);
            }

            // Interface Cemiterio
            if(vis_cemit){
                char cemitText[64];
                snprintf(cemitText, sizeof(cemitText), "CEMITERIO: %d/%d", cemit_num, MAX_CEMITERIO);
                DrawText(cemitText, 6*screenWidth/8, 20, 20, BLACK);
            }

            // Desenha a área do Deck e Cemiterio
            DrawRectangle(deck.deck_area.x, deck.deck_area.y, deck.deck_area.width, deck.deck_area.height, GREEN);
            DrawRectangle(cemiterio.cemit_area.x, cemiterio.cemit_area.y, cemiterio.cemit_area.width, cemiterio.cemit_area.height, BLACK);

            // Desenha as posições do campo e as cartas carregadas nelas
            for(int i=0;i<MAX_POSITION;i++){
                DrawRectangle(areas[i].posicao.x, areas[i].posicao.y, areas[i].posicao.width, areas[i].posicao.height, RED);
                if(areas[i].vazia == false){
                    DrawTexture(areas[i].carregada.textura, areas[i].posicao.x, areas[i].posicao.y, WHITE);
                }
            }

            // Desenha a carta sendo arrastada (se houver)
            if (pull){
                DrawTexture(deck.deck[carta_num].textura, deck.deck[carta_num].posicao.x, deck.deck[carta_num].posicao.y, WHITE);
            }
            if(close == true && areas[id_close].vazia == false){
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 128});
                Texture2D texGrande = areas[id_close].carregada.textura;
                Rectangle source = { 0.0f, 0.0f, (float)texGrande.width, (float)texGrande.height };
                Rectangle pos = { (float)screenWidth/2 - 80, (float)screenHeight/2 - 120, 160, 240 };
                Vector2 normal = { 0, 0 };
                DrawTexturePro(texGrande, source, pos, normal, 0.0f, WHITE);            
            }
            
            if(gui_cemit){
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 128});
                DrawRectangle(screenWidth/2-screenWidth/4, screenHeight/2-screenHeight/4, screenWidth/2+10, screenHeight/2, YELLOW);
                DrawText("Cemiterio", screenWidth/2-screenWidth/4, screenHeight/2-screenHeight/2.5, 60, BLACK);
                int max_coluna;
                for(int i=0; i<20; i++){
                    int linha =(screenHeight/2-screenHeight/4 + screenHeight/8*(i-1)) + 5;
                    if (linha >= screenHeight/2 + screenHeight/4){
                        max_coluna = i-1;
                        break;
                    }
                }
                int row = 1;
                int max_fileira;
                for(int i=0;i<cemit_num; i++){ // realiza o auto-ajuste da largura('beta')
                    int l = i;
                    if(screenWidth/2*row - 45*i <=0){
                        max_fileira = i;
                        row += 1;
                    }
                    if(row != 1){
                        l -= max_fileira;
                    }
                    DrawTexture(cemiterio.cemiterio[i].textura, (screenWidth/2-screenWidth/4+(45*l)) + 5, (screenHeight/2-screenHeight/4 + screenHeight/8*(row-1)) + 5, WHITE);
                }
            }

            //Desenha a trail
            //DrawRectangle(areaRec.x, areaRec.y, areaRec.width, areaRec.height, BLACK);
        EndDrawing();
    }

    //------------------------------------------------------------------------------    
    // De-Initialization / Unload Resources
    //------------------------------------------------------------------------------
    for(int i=0;i<MAX_DECK;i++){
        UnloadTexture(deck.deck[i].textura);
    }     

    CloseWindow();
    return 0;
}