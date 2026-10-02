//----------------------------------------------------------------------------------
// Libraries
//----------------------------------------------------------------------------------
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

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

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void){
    //------------------------------------------------------------------------------
    // Window Initialization
    //------------------------------------------------------------------------------
    const int screenWidth = 1920; //Define a largura da tela
    const int screenHeight = 1080; //Define a altura da tela
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(screenWidth, screenHeight, "a");
    SetWindowPosition(0, 0);
    SetTargetFPS(60); //Define o FPS  
    
    //------------------------------------------------------------------------------
    // Deck & Images Initialization
    //------------------------------------------------------------------------------
    Deck deck; //Instancia o Deck
    deck.area = (Rectangle){screenWidth-120, screenHeight-120, 120, 120}; //Define a area que o deck ocupara
    Deck deckInimigo;
        
    CartaCSV catalogo[MAX_CATALOGO]; //Instancia o catalogo
    Texture2D texturasCarregadas[MAX_CATALOGO]; //Instancia o catalogo de texturas
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

    FOR(i, cartaCarregando) {//Percorre o catalogo pegando os paths e atribuindo as texturas 
        Image img = LoadImage(catalogo[i].caminhoTextura);
        ImageResize(&img, 96, 144);
        texturasCarregadas[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    srand(time(NULL));
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
    FOR(i, MAX_DECK) {
        int escolhido = rand() % MAX_CATALOGO; //Seleciona cartas aleatorias do catalogo e atribui ao deck inimigo
        strncpy(deckInimigo.cartas[i].nome, catalogo[escolhido].nome, sizeof(deckInimigo.cartas[i].nome) - 1);
        deckInimigo.cartas[i].hp = catalogo[escolhido].hp;
        deckInimigo.cartas[i].atk = catalogo[escolhido].atk;
        deckInimigo.cartas[i].cost = catalogo[escolhido].mana;
        deckInimigo.cartas[i].textura = texturasCarregadas[escolhido];
        deckInimigo.cartas[i].id = i + 1;
        deckInimigo.cartas[i].lock = true;
        deckInimigo.cartas[i].shield = false;
    }
     
    //------------------------------------------------------------------------------
    // Game Entities Initialization
    //------------------------------------------------------------------------------    
    Posicao mao[MAX_POSITION] = {0}; //Instancia a mao
    FOR(i, MAX_POSITION)){ //Percorre a mao atribuindo uma posicao para cada posicao, e definindo a posicao como vazia
        mao[i].area = (Rectangle){(screenWidth/2-screenWidth/4) + (136*i)+109, screenHeight/2+screenHeight/64 +18, 96, 144};
        mao[i].vazia = true;
    }
    int quantPuxada = 0; //Intancia a variavel que conta qnts vezes uma carta foi puxada
    int quantMorta = 0; //Variavel q mostra qnts cartas morreram
    Carta arrasto = {0};
    Carta *ataco = NULL;
    bool alterMao = false; //Bool para controlar a visibilidade da mao
    bool compradoTurno = false; //Flag para controlar a comprar por rodada
    bool atacadoTurno = false; //Flag para controlar o ataque por rodada
    bool atacadoTurnoEnemy = false; //Flag para controlar o ataque inimigo
    bool shield = false; //Flag para controlar qnd vc ativa o modo shield
    bool fecharJogo = false;
    int mana = MAX_MANA; //Mana max do jogador
    int hp = 8000;
    int hpInimigo = 8000;
    bool charge = false; //Flag para controlar qnd vc atica o modo charge
    bool chargeTurno = false; //Flag para verificar se já foi realizado um charge no turno


    Posicao campos[MAX_POSITION]; //Instancia as posicoes do campo
    FOR(i, MAX_POSITION){
        campos[i].area.x = i*screenWidth/MAX_POSITION;
        campos[i].area.y = 2.8*screenHeight/4;
        campos[i].area.width = 192;
        campos[i].area.height =288;
        campos[i].vazia = true;
    }

    Posicao camposInimigos[MAX_POSITION]; //Instancia as posicoes do campo inimigo
    FOR(i, MAX_POSITION){
        camposInimigos[i].area.x = i*screenWidth/MAX_POSITION;
        camposInimigos[i].area.y = 0.7  *screenHeight/4;
        camposInimigos[i].area.width = 192;
        camposInimigos[i].area.height =288;
        camposInimigos[i].vazia = true;
    }

    Deck cemiterio; //Estrutura do cemiterio
    cemiterio = (Deck){0};
    cemiterio.area = (Rectangle){screenWidth-120, screenHeight-360, 120, 120};

    Rectangle alterTurno = {screenWidth-120, screenHeight-600, 120, 120}; //Define a opcao de finalizar o turno
    GameState gameState = MENU; //Define o estado inicial como Menu
    TurnState turnState = TURNO_JOGADOR; //Define o estado de turno como meu

    Rectangle jogar = {screenWidth/2 - 264, screenHeight/2 + 48, 528, 120}; //Cria o botao de iniciar o jogo
    Rectangle fechar = {screenWidth/2 - 264, screenHeight/2 + 216, 528, 120}; //Cria o botao para fechar o jogo

    FOR(i, MAX_POSITION){ //Inicializa o deck com 6 cartas iniciais
        mao[i].carregada = deck.cartas[quantPuxada];
        mao[i].carregada.lock = false; 
        camposInimigos[i].carregada = deckInimigo.cartas[quantPuxada];
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
                                    FOR(i, MAX_POSITION){ 
                                        if(mao[i].vazia){ //Verifica se uma posicao da mao esta vazia
                                            Carta disponivel;
                                            int j;
                                            FOR(j, MAX_DECK){
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
                            int ocupadas = 0;
                            FOR(i, MAX_POSITION){ //Verifica qnts posicoes do meu campo estao ocupadas
                                if(!campos[i].vazia){
                                    ocupadas +=1;
                                }
                            }
                            FOR(i, MAX_POSITION){ //Verfica a colisao do mouse com as posicoes da mao e se a UI esta ativa
                                if (CheckCollisionPointRec(GetMousePosition(), mao[i].area) && alterMao){
                                    if(ocupadas == 6){ //Se todas posicoes do meu campo estiverem ocupadas, ao clicar descarto a carta e recupero metade de seu custo
                                        mana += (int)(mao[i].carregada.cost*0.5); //Adiciona a minha mana metade do custo da carta
                                        mao[i].vazia = true;                                        
                                        mao[i].carregada = (Carta){0};
                                    }else if (mao[i].carregada.lock == false){
                                        if (mao[i].vazia == false && arrasto.id == 0){ //Se a colisao for com uma posicao cheia, a posicao e limpa e a uma flag e ativa
                                            if (mana >= mao[i].carregada.cost){
                                                arrasto = mao[i].carregada;
                                                mana -= mao[i].carregada.cost;
                                                mao[i].carregada = (Carta){0};
                                                mao[i].vazia = true;
                                            }
                                        }
                                    }
                                }
                            }
                            FOR(i, MAX_POSITION){//Serve para atacar, pega a carta sobrevoada no clique
                                if (CheckCollisionPointRec(GetMousePosition(), campos[i].area)){
                                    if (campos[i].vazia == false && turnState == TURNO_JOGADOR && atacadoTurno != true){
                                        ataco = &campos[i].carregada;
                                    }
                                }
                            }

                            if (ataco != NULL){ //Verifica se estou no 'modo de ataque'
                                FOR(i, MAX_POSITION){ 
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
                                FOR(i, MAX_POSITION){
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
                        if(IsKeyPressed(KEY_S)){ //Ao clicar S ativa o modo shield
                            bool temShield;
                            FOR(i, MAX_POSITION){ //Verifica se algumas das cartas em campo esta com shield
                                if(campos[i].carregada.shield == true){
                                    temShield = true;
                                    break;
                                } else{
                                    temShield = false;
                                }
                            }
                            if(temShield == false){
                                shield = true;
                            }
                        }
                        if(shield){
                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ //Se o modo shield estiver ativo
                                FOR(i, MAX_POSITION){ //Ao clicar na carta, o atributo shield se ativa, e vc gasta mana
                                    if(CheckCollisionPointRec(GetMousePosition(), campos[i].area)&& campos[i].vazia == false){
                                        if(mana>=campos[i].carregada.cost+1){
                                            campos[i].carregada.shield = true; //Ativa o shield
                                            mana -= (campos[i].carregada.cost+1); //Cobra a mana
                                            shield = false;
                                            break;
                                        }
                                    } else{ //Ao clicar no nada, vc desativa o modo shield
                                        shield = false;
                                    }
                                }
                            }
                        }
                        if(IsKeyPressed(KEY_D)){ //Ativa o charge
                            if(!chargeTurno){
                                charge = true;
                            }
                        }
                        if(charge){
                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                                FOR(i, MAX_POSITION){ //Ao clicar em uma carta com o charge, ela ganha um buff de 1.2 no atk por um round
                                    if(CheckCollisionPointRec(GetMousePosition(), campos[i].area)&& campos[i].vazia == false){
                                        if(mana>=campos[i].carregada.cost-1){                                            
                                            campos[i].carregada.atk *= 1.2;//Altera o atk
                                            if(campos[i].carregada.cost < 2){ //Verifica o custo da carta, para impedir mana negativa
                                                mana -= (campos[i].carregada.cost);
                                            }else{
                                                mana -= (campos[i].carregada.cost-1);
                                            }
                                            chargeTurno = true;
                                            charge = false;
                                            break;
                                        }
                                    } else{
                                        charge = false;
                                    }
                                }
                            }
                        }

                    break;
                    case TURNO_INIMIGO:
                        //--RMB--
                        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
                            FOR(i, MAX_POSITION){//Ao clicar com o RMB sobre uma posicao q tenha uma carta, faz a carta perder 1000 de vida(Funcao teste)
                                if((CheckCollisionPointRec(GetMousePosition(), campos[i].area) && campos[i].vazia == false)){
                                    campos[i].carregada.hp -= 1000;
                                }
                            }       
                        }

                        if (atacadoTurnoEnemy == false){ //Verifica se o inimigo já atacou
                            bool meuOcupado = false;
                            bool inimigoOcupado = false;
                            FOR(i, MAX_POSITION) { //Verifica se as posicoes dos campos estao ocupadas
                                if (campos[i].vazia == false){
                                    meuOcupado= true;
                                }
                                if (camposInimigos[i].vazia == false){
                                    inimigoOcupado = true;
                                }
                            }
                            if (meuOcupado && inimigoOcupado){ //Se no minimo uma posicao de cada lado estiver ocupada, ele ataca um inimigo aleatorio com uma carta aleatoria
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
                                if(campos[escolhaMeu].carregada.shield == false){
                                    campos[escolhaMeu].carregada.hp -= camposInimigos[escolhaInimigo].carregada.atk; //Diminui a vida da minha carta
                                }
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
                            chargeTurno = false;
                            if(mana < MAX_MANA){ //Adiciona mana ao final do round
                                mana +=1;
                            }
                            FOR(i, MAX_POSITION){
                                if(mao[i].vazia == false){
                                    mao[i].carregada.lock = false; //Tira o lock das cartas
                                }
                                if(campos[i].carregada.shield == true){ //Remove o shield 
                                    campos[i].carregada.shield = false;
                                }
                                FOR(j, MAX_CATALOGO){ //Remove o buff do charge, comparando a carta atual com a do catalogo
                                    if(strcmp(campos[i].carregada.nome,catalogo[j].nome) == 0){
                                        campos[i].carregada.atk = catalogo[j].atk;
                                        break;
                                    }
                                }
                            }
                        }    
                    }
                }    
                //--Q--
                if(IsKeyPressed(KEY_Q)){ //Altera a visibilidade da mao
                    alterMao = (!alterMao);     
                }

                //Cards Updates
                FOR(i, MAX_POSITION){
                    if (campos[i].carregada.hp <= 0 && campos[i].vazia == false){ //Se houver uma carta carregada com a vida menor ou igual a 0, ele vai para o cemiterio
                        FOR(j, MAX_DECK){
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
                        mana +=2;
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
                case MENU: { //Desenho do menu inicial
                    //Fundo
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, (Color){20, 25, 35, 255}, (Color){10, 12, 18, 255});

                    //Deconracao
                    Rectangle cardDeco1 = {screenWidth/2 - 528, screenHeight/2 - 312, 192, 288};
                    DrawRectanglePro(cardDeco1, (Vector2){ 96, 144 }, -15.0f, (Color){0, 50, 70, 100});
                    DrawRectangleLinesEx((Rectangle){cardDeco1.x - 96, cardDeco1.y - 144, cardDeco1.width, cardDeco1.height}, 5, (Color){80, 100, 130, 120});

                    Rectangle cardDeco2 = {screenWidth/2 + 528, screenHeight/2 - 312, 192, 288};
                    DrawRectanglePro(cardDeco2, (Vector2){96, 144}, 15.0f, (Color){40, 50, 70, 100});
                    DrawRectangleLinesEx((Rectangle){cardDeco2.x - 96, cardDeco2.y - 144, cardDeco2.width, cardDeco2.height}, 5, (Color){80, 100, 130, 120});

                    //Texto
                    const char *titulo = "SEM NOME";
                    const char *subtitulo = "CARD BATTLE GAME";
                    DrawText(titulo, ((screenWidth / 2) - (MeasureText(titulo, 115) / 2)) + 7, (screenHeight/6) + 7, 115, (Color){0, 0, 0, 180});
                    DrawText(titulo, ((screenWidth / 2) - (MeasureText(titulo, 115) / 2)), (screenHeight/6), 115, GOLD);
                    DrawText(subtitulo, (screenWidth / 2) - (MeasureText(subtitulo, 43) / 2), (screenHeight/6) + 132, 43, LIGHTGRAY);

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
                    DrawRectangleRounded((Rectangle){jogar.x + 10, jogar.y + 10, jogar.width, jogar.height}, 0.25f, 4, (Color){0, 0, 0, 120}); //Desenha uma pequena sombra
                    DrawRectangleRounded(jogar, 0.25f, 4, corBotaoJogar);
                    DrawRectangleRoundedLinesEx(jogar, 0.25f, 4, 5.0f, corBordaJogar); //Desenha uma borda
                    DrawText("JOGAR", jogar.x + (jogar.width / 2) - (MeasureText("JOGAR", 58) / 2), jogar.y + (jogar.height / 2) - 29, 58, WHITE);

                    Color corBotaoFechar;
                    Color corBordaFechar;

                    if (hoverFechar) { //Quando mouse hovering sobre o botao fechar, ele muda de cor
                        corBotaoFechar = (Color){ 140, 40, 50, 255 }; 
                        corBordaFechar = RED;
                    } else {
                        corBotaoFechar = (Color){ 90, 25, 35, 255 };   
                        corBordaFechar = (Color){ 180, 50, 60, 255 };
                    }
                    DrawRectangleRounded((Rectangle){fechar.x + 10, fechar.y + 10, fechar.width, fechar.height}, 0.25f, 4, (Color){0, 0, 0, 120}); //Desenha uma pequena sombra
                    DrawRectangleRounded(fechar, 0.25f, 4, corBotaoFechar);
                    DrawRectangleRoundedLinesEx(fechar, 0.25f, 4, 5.0f, corBordaFechar); //Uma borda
                    DrawText("FECHAR", fechar.x + (fechar.width / 2) - (MeasureText("FECHAR", 58) / 2), fechar.y + (fechar.height / 2) - 29, 58, WHITE);
                } break;

                //--------------------------------------------------------------------------
                // In-Game Draw
                //--------------------------------------------------------------------------
                case JOGANDO:
                    //Fundo
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight,(Color){15, 20, 30, 255}, (Color){8, 10, 15, 255});
                    DrawRectangle(0, 0, screenWidth, screenHeight / 2, (Color){255, 60, 60, 6});
                    DrawRectangle(0, screenHeight / 2, screenWidth, screenHeight/2, (Color){60, 120, 255, 6});
                    DrawLineEx((Vector2){0, screenHeight/2.0f},(Vector2){(float)screenWidth, screenHeight / 2.0f},5.0f, (Color){255, 255, 255, 30});

                    //Deck
                    DrawRectangleRounded((Rectangle){deck.area.x + 10, deck.area.y + 10, deck.area.width, deck.area.height}, 0.15f, 6, (Color){ 20, 25, 35, 200 });
                    DrawRectangleRounded(deck.area, 0.15f, 6, (Color){35, 45, 65, 255});
                    DrawRectangleRoundedLines(deck.area, 0.15f, 6, (Color){80, 110, 160, 255});
                    DrawText("DECK", deck.area.x + deck.area.width / 2 - MeasureText("DECK", 43)/2, deck.area.y + deck.area.height / 2 - 22, 43, (Color){180, 200, 230, 255});

                    // Cemiterio
                    DrawRectangleRounded(cemiterio.area, 0.15f, 6, (Color){20, 20, 20, 255});
                    DrawRectangleRoundedLines(cemiterio.area, 0.15f, 6, (Color){90, 90, 90, 255});

                    //Posicoes
                    FOR(i, MAX_POSITION) {
                        if (campos[i].vazia) {
                            DrawRectangleRounded(campos[i].area, 0.12f, 6, (Color){25, 35, 48, 160});
                            DrawRectangleRoundedLines(campos[i].area, 0.12f, 6, (Color){60, 100, 160, 100});
                        } else {
                            DrawRectangleRounded(campos[i].area, 0.12f, 6, (Color){40, 80, 140, 255});
                            DrawTexturePro(campos[i].carregada.textura, (Rectangle){0, 0, campos[i].carregada.textura.width, campos[i].carregada.textura.height}, (Rectangle){campos[i].area.x, campos[i].area.y, 192.0f, 288.0f}, (Vector2){0, 0}, 0.0f, WHITE);
                            DrawRectangleRoundedLines(campos[i].area, 0.12f, 6, (Color){100, 180, 255, 220});
                        }

                        if (camposInimigos[i].vazia) {
                            DrawRectangleRounded(camposInimigos[i].area, 0.12f, 6, (Color){25, 35, 48, 160});
                            DrawRectangleRoundedLines(camposInimigos[i].area, 0.12f, 6, (Color){60, 100, 160, 100});
                        } else {
                            DrawRectangleRounded(camposInimigos[i].area, 0.12f, 6, (Color){40, 80, 140, 255});
                            DrawTexturePro(camposInimigos[i].carregada.textura, (Rectangle){0, 0, camposInimigos[i].carregada.textura.width, camposInimigos[i].carregada.textura.height}, (Rectangle){camposInimigos[i].area.x, camposInimigos[i].area.y, 192.0f, 288.0f}, (Vector2){0, 0}, 0.0f, WHITE);
                            DrawRectangleRoundedLines(camposInimigos[i].area, 0.12f, 6, (Color){100, 180, 255, 220});
                        }
                        if(campos[i].carregada.shield == true){
                            DrawRectangleRounded(campos[i].area, 0.12f, 6, (Color){255, 255, 0, 80});
                        }
                    }

                    //Mao
                    if (alterMao) {
                        FOR(i, MAX_POSITION) {
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
                    DrawText("PASSAR", alterTurno.x + alterTurno.width/2 - MeasureText("PASSAR", 34)/2,alterTurno.y + alterTurno.height / 2 - 17, 34, WHITE);

                    //Arrasto
                    if (arrasto.id != 0) {
                        DrawTexture(arrasto.textura, arrasto.cordenada.x, arrasto.cordenada.y, WHITE);
                    }

                    //Cards Info
                    char ataque[32];
                    if (ataco != NULL){
                        snprintf(ataque, sizeof(ataque), "ATK: %d", ataco->atk);
                        DrawText(ataque, 336, 24, 48, (Color){255, 0, 0, 155});
                    }


                    //Desenha as infos da carta que seu mouse esta 'sobrevoando'
                    Carta *cartaFocada = NULL;
                    FOR(i, MAX_POSITION){
                        if (CheckCollisionPointRec(GetMousePosition(), campos[i].area)){
                            if (campos[i].vazia == false){
                                cartaFocada = &campos[i].carregada;
                            }
                        }
                    }
                    FOR(i, MAX_POSITION){
                        if (CheckCollisionPointRec(GetMousePosition(), camposInimigos[i].area)){
                            if (camposInimigos[i].vazia == false){
                                cartaFocada = &camposInimigos[i].carregada;
                            }
                        }
                    }
                    FOR(i, MAX_POSITION){
                        if (CheckCollisionPointRec(GetMousePosition(), mao[i].area) && alterMao){
                            if (mao[i].vazia == false){
                                cartaFocada = &mao[i].carregada;
                            }
                        }
                    }
                    if (cartaFocada != NULL) {
                       DrawText(TextFormat("Nome: %s", cartaFocada->nome), 12, 12, 48, (Color){220, 230, 240, 255});
                       DrawText(TextFormat("HP: %d", cartaFocada->hp), 12, 60, 48, (Color){220, 230, 240, 255});
                       DrawText(TextFormat("ATK: %d", cartaFocada->atk), 12, 108, 48, (Color){220, 230, 240, 255});
                       DrawText(TextFormat("Mana: %d", cartaFocada->cost), 12, 156, 48, (Color){220, 230, 240, 255});

                    }

                    DrawText(TextFormat("DECK: %d/%d", MAX_DECK-quantPuxada, MAX_DECK), 6.5*screenWidth/8, 48, 48, (Color){220, 230, 240, 255});
                    DrawText(TextFormat("MANA: %d/%d", mana, MAX_MANA), 6.5*screenWidth/8, 96, 48, (Color){220, 230, 240, 255});

                    char turnoText[64]; //Dsenha em que turno o jogo esta
                    if (turnState == TURNO_JOGADOR) {
                        snprintf(turnoText, sizeof(turnoText), "SEU TURNO");
                    } else {
                        snprintf(turnoText, sizeof(turnoText), "TURNO INIMIGO");
                    }

                    DrawText(turnoText, (screenWidth/2) - (MeasureText(turnoText, 58)/2), 48, 58, (Color){220, 230, 240, 255});
                break;    
            }
        EndDrawing();
    }
    FOR(i, MAX_CATALOGO) {
        UnloadTexture(texturasCarregadas[i]);
    }
        CloseWindow();
    return 0;
}
