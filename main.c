/*******************************************************************************************
*
*   PROJETO: Exemplo 1 - Apostila de Introdução 2D
*   DESCRITIVO: Exemplo prático de jogo de nave com menu interativo
*               e controle modal avançado da tecla ESCAPE.
*
********************************************************************************************/

#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

#define MAXAMMO 10 
#define MAXLUAS 10

// Definição dos estados lógicos de navegação do jogo
typedef enum EstadoJogo {
    MENU,       // Estado que exibe o menu principal e opções do usuário
    JOGANDO    // Estado ativo de simulação de voo com a nave espacial
} EstadoJogo;

// Estrutura modular contendo as propriedades da nave espacial
typedef struct Nave {
    Vector2 posicao;  // Posição bidimensional em floats (x, y) 
    float velocidade;       // Velocidade de deslocamento linear em pixels por quadro
    Color cor;       // Cor de preenchimento da nave no formato RGBA [12, 18]
} Nave;

typedef struct Asteroides {
    Vector2 posicao;  // Posição bidimensional em floats (x, y) 
    Color cor;       // Cor de preenchimento da nave no formato RGBA [12, 18]
} Asteroides;

typedef struct Projeteis{
    Vector2 posicao; // Posição x e y
    Color cor; // Cor
    bool emTela; // Verifica se esta em campo
    float velocidade; // MRU 
} Projeteis;

int ammo;
int pontos = 0;

int main(void) {
    // ---------------------------------------------------------------------------------
    // 1. ETAPA DE INICIALIZAÇÃO E CONFIGURAÇÃO 
    // ---------------------------------------------------------------------------------
    const int larguraTela = 800;   // Definição da largura física da janela do jogo
    const int alturaTela = 600;  // Definição da altura física da janela do jogo

    // Inicialização do motor gráfico e criação da janela do sistema operacional 
    InitWindow(larguraTela, alturaTela, "Apostila Raylib 2D - Nave Espacial");
    SetTargetFPS(60); // Define a taxa constante de quadros em 60 quadros por segundo 

    // DESATIVAÇÃO DA TECLA ESCAPE NATIVA:
    // O comando abaixo impede que pressionar a tecla ESC feche a janela do jogo.
    // Isso é feito para que possamos usar o ESC como comando para voltar para o menu.
    SetExitKey(KEY_P);

    // Inicialização da máquina de estados e variáveis de fluxo de interface
    EstadoJogo estadoAtual = MENU;
    int opcaoSelecionada = 0;       // Representa a opção do menu: 0 = Jogar, 1 = Fechar
    bool fecharJogo = false; // Flag de controle seguro para encerrar o laço

    // Instanciação e parametrização inicial da nave espacial do usuário
    Nave jogador;
    jogador.posicao = (Vector2){ (float)larguraTela / 2.0f, (float)alturaTela / 2.0f }; // Inicia no centro
    jogador.velocidade = 1.0f;   // Define que a nave andará 5.5 pixels a cada quadro de atualização
    jogador.cor = SKYBLUE;   // Cor verde-lima definida na tabela cromática da Raylib 

    // Definir asteroides
    Asteroides luas[MAXLUAS];
    for (int i=0;i<MAXLUAS;i++){
        luas[i].posicao.x = i*larguraTela/MAXLUAS + 42; 
        luas[i].posicao.y = 150;
        luas[i].cor = BLUE;
    }

    // Definir projeteis
    Projeteis tiros[MAXAMMO] = {0}; //Zera todos valores e transforma o bool em false

    // ---------------------------------------------------------------------------------
    // 2. LAÇO PRINCIPAL DO JOGO (GAME LOOP) 
    // ---------------------------------------------------------------------------------
    // O loop continua rodando enquanto a flag de saída for falsa e o usuário não clicar no X da janela [3, 5]
    while (!fecharJogo &&!WindowShouldClose()) {
        
        // =============================================================================
        // ETAPA DE ATUALIZAÇÃO DA LÓGICA DE JOGO (UPDATE) 
        // =============================================================================
        switch (estadoAtual) {
            case MENU: {
                // Captura de entrada discreta (apenas um evento por clique) para navegar no menu 
                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                    opcaoSelecionada = 1; // Seleciona a opção "Fechar"
                }
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                    opcaoSelecionada = 0; // Seleciona a opção "Jogar"
                }

                // Processamento de seleção de opções ao pressionar a tecla ENTER 
                if (IsKeyPressed(KEY_ENTER)) {
                    if (opcaoSelecionada == 0) {
                        // Reseta a nave espacial para o centro da tela antes de começar
                        jogador.posicao = (Vector2){ (float)larguraTela / 2.0f, (float)alturaTela / 2.0f };
                        estadoAtual = JOGANDO; // Muda o estado ativo para a jogabilidade
                    } else if (opcaoSelecionada == 1) {
                        fecharJogo = true; // Define a flag para encerrar o jogo com segurança
                    }
                }
                break;
            }

            case JOGANDO: {
                // Monitora a tecla ESCAPE de forma discreta para retornar ao menu com segurança 
                if (IsKeyPressed(KEY_ESCAPE)) {
                    estadoAtual = MENU; // Altera o estado sem fechar o programa de maneira abrupta [7]
                }

                // Captura contínua de teclas para movimentação linear e suave da nave espacial 
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                    jogador.posicao.x -= jogador.velocidade; // Desloca para a esquerda (subtrai no eixo X) 
                }
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                    jogador.posicao.x += jogador.velocidade; // Desloca para a direita (soma no eixo X) 
                }
                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                    jogador.posicao.y += jogador.velocidade; // Desloca para a direita (soma no eixo X) 
                }
                if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
                    jogador.posicao.y -= jogador.velocidade; // Desloca para cima (subtrai no eixo Y) 
                }
                if (IsKeyPressed(KEY_Z) ) {
                    jogador.velocidade += 1; // Desloca para baixo (soma no eixo Y) 
                }
                if (IsKeyPressed(KEY_X) ) {
                    jogador.velocidade -= 1; // Desloca para baixo (soma no eixo Y) 
                }
                if (IsKeyPressed(KEY_SPACE)){
                    for(int i=0; i<MAXAMMO; i++){
                        if(tiros[i].emTela == false){
                            tiros[i].posicao.x = jogador.posicao.x; //Posição inicial do tiro no eixo x
                            tiros[i].posicao.y = jogador.posicao.y;//Posição inicial do tiro no eixo y
                            tiros[i].cor = RED;//Define a cor
                            tiros[i].emTela = true;//Define que ele esta em campo/tela
                            break;//Quebra o loop qnd um projetil é definido para nao criar varios num mesmo local
                        }
                    }
                }

                // SISTEMA DE COLISÃO BÁSICO (Bordas de Tela):
                // Limita as coordenadas da nave para evitar que o usuário saia do campo visível.
                if (jogador.posicao.x < 15.0f){ 
                    jogador.posicao.x = 15.0f;
                }
                if (jogador.posicao.x > (float)larguraTela - 15.0f) {
                    jogador.posicao.x = (float)larguraTela - 15.0f;
                }
                if (jogador.posicao.y < 20.0f){
                    jogador.posicao.y = 20.0f;}
                if (jogador.posicao.y > (float)alturaTela - 20.0f) {
                    jogador.posicao.y = (float)alturaTela - 20.0f;
                }
                ammo = MAXAMMO;
                for(int i=0;i<MAXAMMO;i++){ //Percorre os tiros
                    if(tiros[i].emTela == true){ // Verifica se o tiro esta na tela
                        tiros[i].posicao.y -=2;//Diminiu a posição y para ele avançar
                        ammo -= 1; // Ao final do loop guarda a informação de quantos tiros disponiveis tal frame tem
                    }
                    if(tiros[i].posicao.y < -3){ // Verifica se ele saiu da tela
                        tiros[i].emTela = false; // Deixa o emTela em false para  conseguir atirar dnv
                    }
                    for(int j=0; j<MAXLUAS; j++){
                        if(CheckCollisionCircles(tiros[i].posicao, 4, luas[j].posicao ,20 )){ //Verifica se o tiro i atingiu uma das luas
                            pontos += 1; // Add um ponto ao contador
                            tiros[i].emTela = false; // Deixa o emTela em false para  conseguir atirar dnv
                            tiros[i].posicao.y = -10; // Faz o tiro sair do campo visivel
                        }

                    }
                }
                
                break;
            }
        }

        // =============================================================================
        // ETAPA DE PROCESSAMENTO GRÁFICO (DRAW) 
        // =============================================================================
        BeginDrawing(); // Inicializa o buffer de desenho ativo 
        ClearBackground(BLACK); // Limpa a tela desenhando uma cor sólida de fundo (Preto Espacial) [9, 19]
        // Desenha a tela atual
        switch (estadoAtual) {
            case MENU: {
                // Renderização do título do menu com centralização básica
                const char* title = "Exemplo 1";
                int titleWidth = MeasureText(title, 44); // Calcula a largura do texto em pixels
                DrawText(title, larguraTela / 2 - titleWidth / 2, 150, 44, GOLD); // [1, 19]

                // Renderização das opções interativas do menu com estilização dinâmica
                if (opcaoSelecionada == 0) {
                    // Estado visual quando "JOGAR" está em foco
                    DrawText("> JOGAR <", larguraTela / 2 - MeasureText("> JOGAR <", 24) / 2, 320, 24, RAYWHITE);
                    DrawText("FECHAR", larguraTela / 2 - MeasureText("FECHAR", 20) / 2, 380, 20, GRAY);
                } else {
                    // Estado visual quando "FECHAR" está em foco
                    DrawText("JOGAR", larguraTela / 2 - MeasureText("JOGAR", 20) / 2, 320, 20, GRAY);
                    DrawText("> FECHAR <", larguraTela / 2 - MeasureText("> FECHAR <", 24) / 2, 380, 24, RAYWHITE);
                }

                // Exibição do rodapé com instruções gerais ao usuário
                const char* footer = "Navegue com W/S ou Setas e selecione com Enter";
                int footerWidth = MeasureText(footer, 14);
                DrawText(footer, larguraTela / 2 - footerWidth / 2, 520, 14, DARKGRAY);
                break;
            }

            case JOGANDO: {
                // HUD: Desenho das informações textuais para orientação de controle
                DrawText("CONTROLES: Use W, A, S, D ou as Setas do teclado", 20, 20, 16, RAYWHITE);
                DrawText("Pressione para retornar ao Menu Principal", 20, 45, 14, GRAY);
                char buffer[16];
                char* myString = "Velocidade: ";
                snprintf(buffer, sizeof(buffer), "%s%f", myString, jogador.velocidade);
                DrawText(buffer, 20, 70, 14, GRAY);
                char ammoText[16];
                snprintf(ammoText, sizeof(ammoText), "Ammo: %d/%d", ammo, MAXAMMO);
                DrawText(ammoText, 6.7*larguraTela/8, 20, 20, YELLOW);
                char pontosText[16];
                snprintf(pontosText, sizeof(pontosText), "Pontos: %d", pontos);
                DrawText(pontosText, 6.7*larguraTela/8, 50, 20, YELLOW);
                for (int i=0;i<MAXLUAS;i++){
                    DrawCircleGradient(luas[i].posicao.x, luas[i].posicao.y, 20.0f, luas[i].cor, PINK);
                }

                for(int i=0;i<MAXAMMO;i++){
                    DrawCircle(tiros[i].posicao.x, tiros[i].posicao.y, 4.0f, GREEN);
                }


                // CÁLCULO DOS VÉRTICES EM SENTIDO ANTI-HORÁRIO (CCW):
                // v1 = Ponta superior da nave [Vértice 1]
                // v2 = Asa inferior esquerda da nave [Vértice 2]
                // v3 = Asa inferior direita da nave [Vértice 3]
                // Esta sequência exata é necessária para o correto preenchimento vetorial do OpenGL.
                Vector2 v1 = { jogador.posicao.x, jogador.posicao.y - 20.0f };
                Vector2 v2 = { jogador.posicao.x - 15.0f, jogador.posicao.y + 15.0f };
                Vector2 v3 = { jogador.posicao.x + 15.0f, jogador.posicao.y + 15.0f };

                // Chamada que envia o triângulo preenchido para processamento na GPU 
                DrawTriangle(v1, v2, v3, jogador.cor);
                break;
            }
        }

        EndDrawing(); // Conclui o desenho e joga as informações para o monitor 
    }

    // ---------------------------------------------------------------------------------
    // 3. ETAPA DE DESINICIALIZAÇÃO E FECHAMENTO 
    // ---------------------------------------------------------------------------------
    CloseWindow(); // Destrói o contexto do dispositivo de vídeo e finaliza a aplicação 

    return 0;
}