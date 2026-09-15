//----------------------------------------------------------------------------------
// Libraries
//----------------------------------------------------------------------------------
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_DECK 60 //Define o maximo de cartas num deck/cemiterio
#define MAX_POSITION 6//Define o maximo de posicoes e cartas na mao

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef struct Carta { //Estruura de uma carta
    Texture2D textura; //Textura da carta
    Vector2 cordenada; //Onde a carta se localiza
    int id; //Id único 
    int hp; //Vida da carta
    int atk; //Ataque da carta
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

typedef enum { //Estrutura de estados
    TURNO_JOGADOR, //Estado do meu turno
    TURNO_INIMIGO //Estado do turno inimigo
} TurnState;

typedef enum {
    MENU,
    JOGANDO
} GameState;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void){
    //------------------------------------------------------------------------------
    // Window Initialization
    //------------------------------------------------------------------------------
    const int screenWidth = 800; //Define a largura da tela
    const int screenHeight = 450; //Define a altura da tela
    InitWindow(screenWidth, screenHeight, "a"); //Inicializa a janela
    SetTargetFPS(60); //Define o FPS  
    
    //------------------------------------------------------------------------------
    // Deck & Images Initialization
    //------------------------------------------------------------------------------
    Deck deck; //Instancia o Deck
    deck.area = (Rectangle){screenWidth-50, screenHeight-50, 50, 50}; //Define a area que o deck ocupara
    Image imagem1 = LoadImage("resources/azul.png"); //Define a imagem 
    Image imagem2 = LoadImage("resources/verde.png"); 
    ImageResize(&imagem1, 40, 60); //Modifica o tamanho da imagem
    ImageResize(&imagem2, 40, 60); 
    Texture2D textura = LoadTextureFromImage(imagem1); //Atribui a imagem a uma textura
    Texture2D textura2 = LoadTextureFromImage(imagem2);
    for(int i=0;i<MAX_DECK;i++){ //Percorre as cartas abribuindo uma textura e id
        if(i%2 == 0){
            deck.cartas[i].textura = textura;
            deck.cartas[i].atk = 1000;
        } else{
            deck.cartas[i].textura = textura2;
            deck.cartas[i].atk = 2000;
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
        mao[i].area = (Rectangle){(screenWidth/2-screenWidth/4) + (56.6*i)+45.3, screenHeight/2+screenHeight/64 +7.5, 40, 60};
        mao[i].vazia = true;
    }
    int quantPuxada = 0; //Intancia a variavel que conta qnts vezes uma carta foi puxada
    int quantMorta = 0; //Variavel q mostra qnts cartas morreram
    Carta arrasto = {0};
    Carta *ataco = NULL;
    bool alterMao = false; //Bool para controlar a visibilidade da mao
    bool compradoTurno = false; //Flag para controlar a comprar por rodada
    bool atacadoTurno = false; //Flag para controlar o ataque por rodada
    bool atacadoTurnoEnemy = false;
    bool fecharJogo = false;

    Posicao campos[MAX_POSITION]; //Instancia as posicoes do campo
    for(int i = 0; i<MAX_POSITION; i++){
        campos[i].area.x = i*screenWidth/MAX_POSITION;
        campos[i].area.y = 2.8*screenHeight/4;
        campos[i].area.width = 80;
        campos[i].area.height =120;
        campos[i].vazia = true;
    }

    Posicao camposInimigos[MAX_POSITION]; //Instancia as posicoes do campo inimigo
    for(int i = 0; i<MAX_POSITION; i++){
        camposInimigos[i].area.x = i*screenWidth/MAX_POSITION;
        camposInimigos[i].area.y = 0.7  *screenHeight/4;
        camposInimigos[i].area.width = 80;
        camposInimigos[i].area.height =120;
        camposInimigos[i].vazia = true;
    }

    Deck cemiterio; //Estrutura do cemiterio
    cemiterio = (Deck){0};
    cemiterio.area = (Rectangle){screenWidth-50, screenHeight-150, 50, 50};

    Rectangle alterTurno = {screenWidth-50, screenHeight-250, 50, 50}; //Define a opcao de finalizar o turno
    GameState gameState = MENU; //Define o estado inicial como Menu
    TurnState turnState = TURNO_JOGADOR; //Define o estado de turno como meu

    Rectangle jogar = {screenWidth/2 - 110, screenHeight/2 + 20, 220, 50}; //Cria o botao de iniciar o jogo
    Rectangle fechar = {screenWidth/2 - 110, screenHeight/2 + 90, 220, 50}; //Cria o botao para fechar o jogo

    for(int i=0; i<MAX_POSITION; i++){ //Inicializa o deck com 6 cartas iniciais
        mao[i].carregada = deck.cartas[quantPuxada]; 
        camposInimigos[i].carregada = deck.cartas[quantPuxada];
        camposInimigos[i].vazia = false;
        deck.cartas[quantPuxada] = (Carta){0};        
        mao[i].vazia = false;
        quantPuxada += 1;
    }   

    while(!WindowShouldClose() && !fecharJogo){
        Vector2 mousePosition = GetMousePosition(); //Pega a posicao do mouse
        switch(gameState){
            case MENU:
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ //Se o botao de jogar for apertado muda o estado para Jogando
                        if ((CheckCollisionPointRec(GetMousePosition(), jogar))){
                            gameState = JOGANDO; //Altera o estado

                        }
                        if ((CheckCollisionPointRec(GetMousePosition(), fechar))){ //Se o botao de fechar for apertado fecha o jogo
                            fecharJogo = true;//Altera a flag
                        }
                    }
                break;
            case JOGANDO:
                switch (turnState) {
                    case TURNO_JOGADOR:
                        if (arrasto.id != 0){ //Altera a posicao de uma carta que esta sendo arrastada
                            arrasto.cordenada = (Vector2){mousePosition.x-20, mousePosition.y-30};
                        }
                        //--LMB--
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ //Verifica Click LMB(Left Mouse Button)
                            if(quantPuxada < MAX_DECK){ //Nao permite que vc puxe mais do que tem
                                if (CheckCollisionPointRec(GetMousePosition(), deck.area) && compradoTurno == false){ //Checa a colisão entre o mouse e deck
                                    for(int i=0; i<MAX_POSITION; i++){ 
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
                                            compradoTurno = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            for (int i=0;i<MAX_POSITION; i++){ //Verfica a colisao do mouse com as posicoes da mao e se a UI esta ativa
                                if (CheckCollisionPointRec(GetMousePosition(), mao[i].area) && alterMao){
                                    if (mao[i].vazia == false && arrasto.id == 0){ //Se a colisao for com uma posicao cheia, a posicao e limpa e a uma flag e ativa
                                        arrasto = mao[i].carregada;
                                        mao[i].carregada = (Carta){0};
                                        mao[i].vazia = true;
                                    }
                                }
                            }
                            for (int i=0; i<MAX_POSITION; i++){
                                if (CheckCollisionPointRec(GetMousePosition(), campos[i].area)){
                                    if (campos[i].vazia == false && turnState == TURNO_JOGADOR && atacadoTurno != true){
                                        ataco = &campos[i].carregada;
                                    }
                                }
                            }

                            if (ataco != NULL){ //Verifica se estou no 'modo de ataque'
                                for (int i=0; i<MAX_POSITION; i++){ 
                                    if (CheckCollisionPointRec(GetMousePosition(), camposInimigos[i].area)){//Checa a colisao com uma posicao inimiga ocupada
                                        if (camposInimigos[i].vazia == false){
                                            camposInimigos[i].carregada.hp -= ataco->atk; //Diminui a vida da carta inimiga
                                            ataco = NULL;
                                            atacadoTurno = true;
                                        }
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
                    break;
                    case TURNO_INIMIGO:
                        //--RMB--
                        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
                            for(int i = 0; i<MAX_POSITION; i++){//Ao clicar com o RMB sobre uma posicao q tenha uma carta, faz a carta perder 1000 de vida(Funcao teste)
                                if((CheckCollisionPointRec(GetMousePosition(), campos[i].area) && campos[i].vazia == false)){
                                    campos[i].carregada.hp -= 1000;
                                }
                            }       
                        }

                        if (atacadoTurnoEnemy == false){ //Verifica se o inimigo já atacou
                            bool meuOcupado = false;
                            bool inimigoOcupado = false;
                            for (int i = 0; i < 6; i++) { //Verifica se as posicoes dos campos estao ocupadas
                                if (campos[i].vazia == false){
                                    meuOcupado= true;
                                }
                                if (camposInimigos[i].vazia == false){
                                    inimigoOcupado = true;
                                }
                            }
                            if (meuOcupado && inimigoOcupado){ //Se no minimo uma posicao de cada lado estiver ocupada, ele ataca um inimigo aleatorio com uma carta aleatoria
                                srand(time(NULL));
                                int escolhaMeu;
                                int escolhaInimigo;
                                while (true){
                                    escolhaMeu = rand() %6; //Sorteia uma posicao
                                    if(campos[escolhaMeu].vazia == false){ //Verifica se a posicao sorteada esta ocupada
                                        break;
                                    }
                                }
                                while (true){
                                    escolhaInimigo = rand() %6; //Sorteia uma posicao
                                    if(camposInimigos[escolhaInimigo].vazia == false){ //Verifica se a posicao sorteada esta ocupada
                                        break;
                                    }
                                }
                                campos[escolhaMeu].carregada.hp -= camposInimigos[escolhaInimigo].carregada.atk; //Diminui a vida da minha carta
                                atacadoTurnoEnemy = true; //Encerra o ataque inimigo
                            }
                        }

                    break;
                }
                //--------------------------------------------------------------------------
                // Global features    
                //--------------------------------------------------------------------------    

                //--LMB--
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ //Alterna entre os turnos ao clicar no botao de alterar turno
                    if((CheckCollisionPointRec(GetMousePosition(), alterTurno))){
                        if(turnState == TURNO_JOGADOR){
                            turnState = TURNO_INIMIGO; //Finalizo meu turno, e passo para o inimigo
                            atacadoTurnoEnemy = false;
                        } else {
                            turnState = TURNO_JOGADOR; //Inimigo finaliza o turno
                            compradoTurno = false;  //Me permite comprar outra carta
                            atacadoTurno = false;
                        }    
                    }
                }    
                //--Q--
                if(IsKeyPressed(KEY_Q)){ //Altera a visibilidade da mao
                    alterMao = (!alterMao);     
                }

                //Cards Updates
                for (int i=0; i<MAX_POSITION; i++){
                    if (campos[i].carregada.hp <= 0 && campos[i].vazia == false){ //Se houver uma carta carregada com a vida menor ou igual a 0, ele vai para o cemiterio
                        for(int j = 0; j<MAX_DECK; j++){
                            if(cemiterio.cartas[j].id == 0){
                                cemiterio.cartas[j] = campos[i].carregada; //Carta morta e atribuida ao cemiterio
                                campos[i].carregada = (Carta){0}; //A posicao dela e limpa
                                campos[i].vazia = true; //A posicao e definida como vazia
                                quantMorta += 1;
                                break;
                            }
                        }
                        break;
                    }
                    if (camposInimigos[i].carregada.hp <= 0 && camposInimigos[i].vazia == false){ //Se houver uma carta carregada com a vida menor ou igual a 0, ele vai para o cemiterio
                        camposInimigos[i].carregada = (Carta){0}; //A posicao dela e limpa
                        camposInimigos[i].vazia = true; //A posicao e definida como vazia
                        break;
                    }
                }  
            break;    
        }   
    
        //--------------------------------------------------------------------------
        // 2. Rendering
        //--------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(WHITE);
            switch(gameState){
                //--------------------------------------------------------------------------
                // Menu Draw
                //--------------------------------------------------------------------------
                case MENU: //Desenho do menu inicial
                    //Fundo
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, (Color){20, 25, 35, 255}, (Color){10, 12, 18, 255});

                    //Deconracao
                    Rectangle cardDeco1 = {screenWidth/2 - 220, screenHeight/2 - 130, 80, 120};
                    DrawRectanglePro(cardDeco1, (Vector2){ 40, 60 }, -15.0f, (Color){0, 50, 70, 100});
                    DrawRectangleLinesEx((Rectangle){cardDeco1.x - 40, cardDeco1.y - 60, cardDeco1.width, cardDeco1.height}, 2, (Color){80, 100, 130, 120});

                    Rectangle cardDeco2 = {screenWidth/2 + 220, screenHeight/2 - 130, 80, 120};
                    DrawRectanglePro(cardDeco2, (Vector2){40, 60}, 15.0f, (Color){40, 50, 70, 100});
                    DrawRectangleLinesEx((Rectangle){cardDeco2.x - 40, cardDeco2.y - 60, cardDeco2.width, cardDeco2.height}, 2, (Color){80, 100, 130, 120});

                    //Texto
                    const char *titulo = "SEM NOME";
                    const char *subtitulo = "CARD BATTLE GAME";
                    DrawText(titulo, ((screenWidth / 2) - (MeasureText(titulo, 48) / 2)) + 3, (screenHeight/6) + 3, 48, (Color){0, 0, 0, 180});
                    DrawText(titulo, ((screenWidth / 2) - (MeasureText(titulo, 48) / 2)), (screenHeight/6), 48, GOLD);
                    DrawText(subtitulo, (screenWidth / 2) - (MeasureText(subtitulo, 18) / 2), (screenHeight/6) + 55, 18, LIGHTGRAY);

                    //hover
                    bool hoverJogar = CheckCollisionPointRec(mousePosition, jogar);
                    bool hoverFechar = CheckCollisionPointRec(mousePosition, fechar);

                    //Botoes
                    Color corBotaoJogar;
                    Color corBordaJogar;
                    if (hoverJogar) { //Quando mouse hovering sobre o botao jogar, ele muda de cor
                        corBotaoJogar = (Color){ 46, 117, 89, 255 };  
                        corBordaJogar = GREEN;
                    } else {
                        corBotaoJogar = (Color){ 30, 80, 60, 255 };    
                        corBordaJogar = (Color){ 40, 160, 100, 255 };
                    }
                    DrawRectangleRounded((Rectangle){jogar.x + 4, jogar.y + 4, jogar.width, jogar.height}, 0.25f, 4, (Color){0, 0, 0, 120}); //Desenha uma pequena sombra
                    DrawRectangleRounded(jogar, 0.25f, 4, corBotaoJogar);
                    DrawRectangleRoundedLinesEx(jogar, 0.25f, 4, 2.0f, corBordaJogar); //Desenha uma borda
                    DrawText("JOGAR", jogar.x + (jogar.width / 2) - (MeasureText("JOGAR", 24) / 2), jogar.y + (jogar.height / 2) - 12, 24, WHITE);

                    Color corBotaoFechar;
                    Color corBordaFechar;

                    if (hoverFechar) { //Quando mouse hovering sobre o botao fechar, ele muda de cor
                        corBotaoFechar = (Color){ 140, 40, 50, 255 }; 
                        corBordaFechar = RED;
                    } else {
                        corBotaoFechar = (Color){ 90, 25, 35, 255 };   
                        corBordaFechar = (Color){ 180, 50, 60, 255 };
                    }
                    DrawRectangleRounded((Rectangle){fechar.x + 4, fechar.y + 4, fechar.width, fechar.height}, 0.25f, 4, (Color){0, 0, 0, 120}); //Desenha uma pequena sombra
                    DrawRectangleRounded(fechar, 0.25f, 4, corBotaoFechar);
                    DrawRectangleRoundedLinesEx(fechar, 0.25f, 4, 2.0f, corBordaFechar); //Uma borda
                    DrawText("FECHAR", fechar.x + (fechar.width / 2) - (MeasureText("FECHAR", 24) / 2), fechar.y + (fechar.height / 2) - 12, 24, WHITE);
                break;

                //--------------------------------------------------------------------------
                // In-Game Draw
                //--------------------------------------------------------------------------
                case JOGANDO:
                    //Fundo
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight,(Color){15, 20, 30, 255}, (Color){8, 10, 15, 255});
                    DrawRectangle(0, 0, screenWidth, screenHeight / 2, (Color){255, 60, 60, 6});
                    DrawRectangle(0, screenHeight / 2, screenWidth, screenHeight/2, (Color){60, 120, 255, 6});
                    DrawLineEx((Vector2){0, screenHeight/2.0f},(Vector2){(float)screenWidth, screenHeight / 2.0f},2.0f, (Color){255, 255, 255, 30});

                    //Deck
                    DrawRectangleRounded((Rectangle){deck.area.x + 4, deck.area.y + 4, deck.area.width, deck.area.height}, 0.15f, 6, (Color){ 20, 25, 35, 200 });
                    DrawRectangleRounded(deck.area, 0.15f, 6, (Color){35, 45, 65, 255});
                    DrawRectangleRoundedLines(deck.area, 0.15f, 6, (Color){80, 110, 160, 255});
                    DrawText("DECK", deck.area.x + deck.area.width / 2 - MeasureText("DECK", 18)/2, deck.area.y + deck.area.height / 2 - 9, 18, (Color){180, 200, 230, 255});

                    // Cemiterio
                    DrawRectangleRounded(cemiterio.area, 0.15f, 6, (Color){20, 20, 20, 255});
                    DrawRectangleRoundedLines(cemiterio.area, 0.15f, 6, (Color){90, 90, 90, 255});

                    //Posicoes
                    for (int i = 0; i < MAX_POSITION; i++) {
                        if (campos[i].vazia) {
                            DrawRectangleRounded(campos[i].area, 0.12f, 6, (Color){25, 35, 48, 160});
                            DrawRectangleRoundedLines(campos[i].area, 0.12f, 6, (Color){60, 100, 160, 100});
                        } else {
                            DrawRectangleRounded(campos[i].area, 0.12f, 6, (Color){40, 80, 140, 255});
                            DrawTexturePro(campos[i].carregada.textura, (Rectangle){0, 0, campos[i].carregada.textura.width, campos[i].carregada.textura.height}, (Rectangle){campos[i].area.x, campos[i].area.y, 80.0f, 120.0f}, (Vector2){0, 0}, 0.0f, WHITE);
                            DrawRectangleRoundedLines(campos[i].area, 0.12f, 6, (Color){100, 180, 255, 220});
                        }

                        if (camposInimigos[i].vazia) {
                            DrawRectangleRounded(camposInimigos[i].area, 0.12f, 6, (Color){25, 35, 48, 160});
                            DrawRectangleRoundedLines(camposInimigos[i].area, 0.12f, 6, (Color){60, 100, 160, 100});
                        } else {
                            DrawRectangleRounded(camposInimigos[i].area, 0.12f, 6, (Color){40, 80, 140, 255});
                            DrawTexturePro(camposInimigos[i].carregada.textura, (Rectangle){0, 0, camposInimigos[i].carregada.textura.width, camposInimigos[i].carregada.textura.height}, (Rectangle){camposInimigos[i].area.x, camposInimigos[i].area.y, 80.0f, 120.0f}, (Vector2){0, 0}, 0.0f, WHITE);
                            DrawRectangleRoundedLines(camposInimigos[i].area, 0.12f, 6, (Color){100, 180, 255, 220});
                        }
                    }

                    //Mao
                    if (alterMao) {
                        for (int i = 0; i < MAX_POSITION; i++) {
                            DrawRectangleRounded(mao[i].area, 0.12f, 6, (Color){20, 28, 40, 180});
                            DrawRectangleRoundedLines(mao[i].area, 0.12f, 6, (Color){140, 180, 240, 100});
                            if (!mao[i].vazia) {
                                DrawTexture(mao[i].carregada.textura, mao[i].area.x, mao[i].area.y, WHITE);
                            }
                        }
                    }

                    //Finalizar turno
                    bool hoverPassar = CheckCollisionPointRec(GetMousePosition(), alterTurno);
                    Color corBotaoPassar;
                    if (hoverPassar) {
                        corBotaoPassar = (Color){200, 60, 60, 255};
                    } else {
                        corBotaoPassar = (Color){140, 40, 40, 255};
                    }
                    DrawRectangleRounded(alterTurno, 0.25f, 6, corBotaoPassar);
                    DrawRectangleRoundedLines(alterTurno, 0.25f, 6, (Color){255, 140, 140, 255});
                    DrawText("PASSAR", alterTurno.x + alterTurno.width/2 - MeasureText("PASSAR", 14)/2,alterTurno.y + alterTurno.height / 2 - 7, 14, WHITE);

                    //Arrasto
                    if (arrasto.id != 0) {
                        DrawTexture(arrasto.textura, arrasto.cordenada.x, arrasto.cordenada.y, WHITE);
                    }

                    //Cards Info
                    char ataque[32];
                    if (ataco != NULL){
                        snprintf(ataque, sizeof(ataque), "ATK: %d", ataco->atk);
                        DrawText(ataque, 140, 10, 20, (Color){255, 0, 0, 155});
                    }


                    char nomeCarta[64]; //Desenha as infos da carta que seu mouse esta 'sobrevoando'
                    char vidaCarta[16];
                    char ataqueCarta[16];
                    Carta *cartaFocada = NULL;
                    for (int i=0; i<MAX_POSITION; i++){
                        if (CheckCollisionPointRec(GetMousePosition(), campos[i].area)){
                            if (campos[i].vazia == false){
                                cartaFocada = &campos[i].carregada;
                            }
                        }
                    }
                    for (int i=0; i<MAX_POSITION; i++){
                        if (CheckCollisionPointRec(GetMousePosition(), camposInimigos[i].area)){
                            if (camposInimigos[i].vazia == false){
                                cartaFocada = &camposInimigos[i].carregada;
                            }
                        }
                    }
                    for (int i=0; i<MAX_POSITION; i++){
                        if (CheckCollisionPointRec(GetMousePosition(), mao[i].area) && alterMao){
                            if (mao[i].vazia == false){
                                cartaFocada = &mao[i].carregada;
                            }
                        }
                    }
                    if (cartaFocada != NULL){
                        snprintf(nomeCarta, sizeof(nomeCarta), "Nome: %d", cartaFocada->id);
                        snprintf(vidaCarta, sizeof(vidaCarta), "HP: %d", cartaFocada->hp);
                        snprintf(ataqueCarta, sizeof(ataqueCarta), "ATK: %d", cartaFocada->atk);
                        DrawText(nomeCarta, 5, 5, 20, (Color){220, 230, 240, 255});
                        DrawText(vidaCarta, 5, 25, 20, (Color){220, 230, 240, 255});
                        DrawText(ataqueCarta, 5, 45, 20, (Color){220, 230, 240, 255});
                    }

                    char deckText[64]; //Textos de informacoes de qnts de cartas no deck e cemiterio
                    char cemiterioText[64];
                    snprintf(cemiterioText, sizeof(cemiterioText), "CEMITERIO: %d/%d", quantMorta, MAX_DECK);
                    snprintf(deckText, sizeof(deckText), "DECK: %d/%d", MAX_DECK-quantPuxada, MAX_DECK);
                    DrawText(deckText, 6.5*screenWidth/8, 20, 20, (Color){220, 230, 240, 255});
                    DrawText(cemiterioText, 6*screenWidth/8, 40, 20, (Color){220, 230, 240, 255});

                    char turnoText[64]; //Dsenha em que turno o jogo esta
                    if (turnState == TURNO_JOGADOR) {
                        snprintf(turnoText, sizeof(turnoText), "SEU TURNO");
                    } else {
                        snprintf(turnoText, sizeof(turnoText), "TURNO INIMIGO");
                    }

                    DrawText(turnoText, (screenWidth/2) - (MeasureText(turnoText, 24)/2), 20, 24, (Color){220, 230, 240, 255});
                break;    
            }
        EndDrawing();
    }

    UnloadTexture(textura);
    UnloadTexture(textura2);
    CloseWindow();
    return 0;
}
