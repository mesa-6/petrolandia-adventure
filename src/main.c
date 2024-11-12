// INCLUSÃO DE BIBLIOTECA
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DEFININDO AS CONSTANTES QUE SERÃO UTILIZADAS NO JOGO
#define NUM_MAX_OBJETOS 13 // Número máximo de obstáculos - (Pedras)
#define PRIMEIRA_ONDA 7 // Número de obstáculos na primeira onda
#define SEGUNDA_ONDA 10 // Número de obstáculos na segunda onda
#define TERCEIRA_ONDA 13 // Número de obstáculos na terceira onda
#define NUM_MAX_BANHISTA 5 // Número máximo de banhistas que aparecem simultaneamente
#define MAX_RANKING 100 // Quantidade máxima de registros no arquivo de ranking

typedef enum { PRIMEIRA = 0, SEGUNDA, TERCEIRA } OndaObstaculo;  // OndaObstaculo é um estado que caracteriza em qual onda de obstáculos o jogador está
typedef enum { MENU = 0, PLAY, GAME_OVER } GameState; // GameState é um estado que basicamente diz em que tela o jogador está

// ESTRUTURAS DE DADOS
typedef struct Barco{ // Estrutura que representa o barco do jogador
	Rectangle rec;
	Vector2 speed;
} Barco;

typedef struct Obstaculo{ // Estrutura que representa os obstáculos (Pedras)
	Rectangle rec;
	Vector2 speed;
	bool active;
} Obstaculo;

typedef struct ListObj{ // Lista encadeada de obstáculos
	Obstaculo obstaculo;
	struct ListObj *prox;
} ListObj;

typedef struct Banhista{ // Estrutura que representa os banhistas
	Rectangle rec;
	Vector2 speed;
	bool active;
} Banhista;

typedef struct ListBanhista{ // Lista encadeada de banhistas que aparecem na tela
	Banhista banhista;
	struct Listbanhista *prox;
} ListBanhista;

typedef struct BanhistaColetados{ // Lista encadeada de banhistas coletados
	Banhista banhista;
	struct Listbanhista *prox;
} BanhistaColetados;

typedef struct Player { // Estrutura que representa o jogador - VAI PRO RANKING
	char nome[20];
	int score;
	int banhistas;
} Player;

// DECLARAÇÃO DAS HEADS E TAILS DAS LISTAS DE OBSTÁCULOS, BANHISTAS E BANHISTAS COLETADOS
ListObj *head = NULL;
ListObj *tail = NULL;

ListBanhista *headBanhista = NULL;
ListBanhista *tailBanhista = NULL;

BanhistaColetados *headBanhistaColetados = NULL;
BanhistaColetados *tailBanhistaColetados = NULL;

// Definição da dimensionamento da tela
static const int screenWidth = 1000;
static const int screenHeight = 749;

// DECLARAÇÃO DE VARIÁVEIS
static char nick[20] = {0}; // Nickname do jogador
static int nickIndex = 0; // Quantidade de caracteres digitados no nick
static bool enteringNick = false; // Flag que indica se o jogador está digitando o nick
static GameState currentGameState = MENU;  // Inicializa o jogo no menu

int score = 0; // Pontuação do jogador
int vida = 10; // Vida inicial do jogador
int banhistaSalvos = 0; // Quantidade de banhistas salvos
static bool gameOver = false; // Variável que indica o fim do jogo
static float timerBanhista = 0; // Timer para controlar a aparição de banhistas
static bool victory = false; // Variável que indica a vitória do jogador

static Barco barco = { 0 }; // Inicializa o barco do jogador

static OndaObstaculo wave = { 0 }; // Inicializa a onda de obstáculos como a primeira

// Controlam o texto de onda atual
static float alpha = 0.0f; 
static bool smooth = false;

static int activeObstaculos = 0; // Número de obstáculos ativos na tela

// Inicializando as fotos do jogo
Texture2D moeda;
Texture2D MenuInicial;
Texture2D PedraTextura;
Texture2D Barcotextura;
Texture2D FundoVitoria;
Texture2D coracaoTextura;
Texture2D CenarioTexture;
Texture2D BanhistaTextura;

// Protótipos das funções
static void InitGame(void);
static void limpaObstaculos(ListObj **head, ListObj **tail);
static void limparBanhistas(ListBanhista **head, ListBanhista **tail);
static void limparBanhistasColetados(BanhistaColetados **head, BanhistaColetados **tail);
static void inserirobstaculos(ListObj **head, ListObj **tail);
static void inserirBanhista(ListBanhista **head, ListBanhista **tail);
static void inserirBanhistaColetados(BanhistaColetados **head, BanhistaColetados **tail);
static int countBanhistasColetados(BanhistaColetados *head);
static void removerBanhistaColetados(BanhistaColetados **head, BanhistaColetados **tail);
static void DrawMenu(void);
static void UpdateMenu(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);
static void DrawGameOverScreen(void);
static void salvarRanking(const char* nome, int score, int numBanhistas);

// MAIN
int main(void) {
	//  Inicializa a janela com as larguras e alturas definidas + título
	InitWindow(screenWidth, screenHeight, "Petrolândia Adventure");

	// Inicializa o jogo
	InitGame();
	
	// Define a taxa de atualização da tela (FRAMES POR SEGUNDO)
	SetTargetFPS(60);

	// Loop principal do jogo
	while (!WindowShouldClose()) {
		// Atualiza e desenha o frame
		UpdateDrawFrame();
	}
	
	// Descarrega o jogo
	UnloadGame();

	// Fecha a janela
	CloseWindow();
	return 0;
}

// INIT GAME
void InitGame(void) {
	limpaObstaculos(&head, &tail); // Limpa a lista de obstáculos
	limparBanhistas(&headBanhista, &tailBanhista); // Limpa a lista de banhistas que aparecem na tela
	limparBanhistasColetados(&headBanhistaColetados, &tailBanhistaColetados); // Limpa a lista de banhistas coletados
	
	// Carraga todas as texturas do jogo
	Barcotextura = LoadTexture("resources/Barco.png");
	CenarioTexture = LoadTexture("resources/Cenario.png");
	MenuInicial = LoadTexture("resources/MenuInical.png");
	PedraTextura = LoadTexture("resources/pedra.png");
	BanhistaTextura = LoadTexture("resources/Pessoa4.png");
	FundoVitoria = LoadTexture("resources/fim.png");
	coracaoTextura = LoadTexture("resources/Coracao.png");
	moeda = LoadTexture ("resources/moeda.png");

	// Inicializa as variáveis do jogo --> Necessário para reiniciar o jogo com as variáveis zeradas
	score = 0;
	vida = 100;
	alpha = 0;
	smooth = false;
	wave = PRIMEIRA;
	victory = false;
	gameOver = false;
	barco.rec.x = 20;
	barco.rec.y = 50;
	barco.speed.x = 4;
	barco.speed.y = 4;
	banhistaSalvos = 0;
	activeObstaculos = PRIMEIRA_ONDA;
	barco.rec.width = Barcotextura.width;
	barco.rec.height = Barcotextura.height - 30;

	// Inicializa a lista de obstáculos
	for (int i = 0; i < activeObstaculos; i++) {
		inserirobstaculos(&head, &tail);
	}

	// Inicializa a lista de banhistas que aparecem na tela
	for (int i = 0; i < NUM_MAX_BANHISTA; i++) {
		inserirBanhista(&headBanhista, &tailBanhista);
	}
}

// LIMPA OBSTÁCULOS
void limpaObstaculos(ListObj **head, ListObj **tail) {
	// Recebe o head e a tail da lista de obstáculos e libera a memória

    if (*head == NULL){
		return;
	} else {
		ListObj *aux = *head;
    	ListObj *prox;

    	do { // Percorre a lista liberando a memória - LISTA CIRCULAR
        	prox = aux->prox;
        	free(aux);
        	aux = prox;
    	} while (aux != *head);

		// Atualiza o head e a tail para NULL
		*head = NULL;
		*tail = NULL;
	}
}

// LIMPA BANHISTAS DA TELA
void limparBanhistas(ListBanhista **head, ListBanhista **tail){
	// Recebe o head e a tail da lista de banhistas e libera a memória
	if (*head == NULL) {
		return;
	} else {
		ListBanhista *aux = *head;
		ListBanhista *prox = *head;
		while (aux != NULL) { // Percorre a lista liberando a memória - LISTA SIMPLESMENTE ENCADEADA
			prox = prox->prox;
			free(aux);
			aux = prox;
		}
		// Atualiza o head e a tail para NULL
		*head = NULL;
		*tail = NULL;
	}
}

// LIMPA BANHISTAS COLETADOS
void limparBanhistasColetados(BanhistaColetados **head, BanhistaColetados **tail) {
	// Recebe o head e a tail da lista de banhistas coletados e libera a memória
	if (*head == NULL) {
		return;
	} else {
		BanhistaColetados *aux = *head;
		BanhistaColetados *prox = *head;
		while (aux != NULL) { // Percorre a lista liberando a memória - LISTA SIMPLESMENTE ENCADEADA
			prox = prox->prox;
			free(aux);
			aux = prox;
		}
		// Atualiza o head e a tail para NULL
		*head = NULL;
		*tail = NULL;
	}
}

// INSERIR OBSTÁCULOS
void inserirobstaculos (ListObj **head, ListObj **tail) {
	// Lista Circular
  	ListObj *novo = (ListObj *) malloc(sizeof(ListObj)); // Aloca memória para um novo obstáculo

	if(novo != NULL) { 
		// Características do obstáculo
		novo->prox = *head;
		novo->obstaculo.speed.x = 5;
		novo->obstaculo.speed.y = 5;
		novo->obstaculo.active = true;
		novo->obstaculo.rec.width = PedraTextura.width - 45;
		novo->obstaculo.rec.height = PedraTextura.height - 45;
		novo->obstaculo.rec.y = GetRandomValue(470, 700); // Posição aleatória na tela em y
		novo->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000); // Posição aleatória na tela em x
		
		if (*head==NULL) {
			*head = *tail = novo;
			(*tail)->prox = novo;
		} else {
			(*tail)->prox = novo;
			novo->prox = *head;
			*tail = novo;
		}
	}
}

// INSERIR BANHISTAS
void inserirBanhista (ListBanhista **head, ListBanhista **tail) {
	// Lista Simplesmente Encadeada
    ListBanhista *nova = (ListBanhista *)malloc(sizeof(ListBanhista)); // Aloca memória para um novo banhista
    
	if (nova != NULL){
		// Características do banhista
        nova->prox = NULL;
        nova->banhista.speed.x = 5;
        nova->banhista.speed.y = 5;
        nova->banhista.active = true;
        nova->banhista.rec.width = 15;
        nova->banhista.rec.height = 15;
        nova->banhista.rec.y = GetRandomValue(470, 700); // Posição aleatória na tela em y
        nova->banhista.rec.x = GetRandomValue(screenWidth, screenWidth + 1000); // Posição aleatória na tela em x

        if (*head == NULL) {
            *head = nova;
            *tail = nova;
        } else {
            (*tail)->prox = nova;
            *tail = nova;
        }
    }
}

// ATUALIZAR E DESENHAR O FRAME
void UpdateDrawFrame(void) {
	// Essa função se baseia no estado atual do jogo para renderizar a tela
    switch (currentGameState) {
        case MENU: // Se o estado atual for MENU
            UpdateMenu(); // Atualiza o menu
            DrawMenu(); // Desenha o menu
            break;
        case PLAY:
            UpdateGame(); // Atualiza o jogo
            DrawGame(); // Desenha o jogo
            break;
		case GAME_OVER:
			DrawGameOverScreen(); // Desenha a tela de game over
			if (IsKeyPressed(KEY_ENTER)) { // Se o jogador pressionar ENTER
				currentGameState = PLAY; // Troca o estado para PLAY
			} else if (IsKeyPressed(KEY_M)) { // Se o jogador pressionar M
				currentGameState = MENU; // Troca o estado para MENU
			}
            break;
        default:
            break;
    }
}

// ATUALIZAR MENU
void UpdateMenu(void) {
    if (IsKeyPressed(KEY_ENTER)) { // Se o jogador pressionar ENTER
        if (!enteringNick) {
            enteringNick = true; // Inicia a entrada do nick
        } else if (nickIndex > 0) { // Confirma o nick e inicia o jogo se já tiver digitado algo
            enteringNick = false;
            currentGameState = PLAY; // Inicia o jogo trocando o estado para PLAY
        }
    }

    if (enteringNick) { // Se o jogador estiver digitando o nick
        int key = GetKeyPressed(); // Captura a tecla pressionada

        // Captura de texto
        if (key >= 32 && key <= 125 && nickIndex < 19) { // Letras, números e símbolos
            nick[nickIndex++] = (char) key; // Transforma o código ASCII em caractere e insere no nick
            nick[nickIndex] = '\0'; // Adiciona o caractere nulo ao final da string
        }

        // Apagar último caractere
        if (IsKeyPressed(KEY_BACKSPACE) && nickIndex > 0) {
            nick[--nickIndex] = '\0';
        }
    } else if (IsKeyPressed(KEY_ESCAPE)) { // Se o jogador pressionar ESC
        CloseWindow(); // Fecha a janela do jogo
    } else if (IsKeyPressed(KEY_R)) { // Se o jogador pressionar R
        currentGameState = GAME_OVER; // Troca o estado para GAME_OVER
    }
}

// DESENHAR MENU
void DrawMenu(void) {
    BeginDrawing(); // Inicia o desenho
    
    DrawTexture(MenuInicial, 0, 0, WHITE); // Desenha a imagem de fundo do menu
    
	// Desenha os textos do menu
    DrawText("PETROLÂNDIA ADVENTURE", 20, 20, 45, DARKBLUE); 
    DrawText("ENTER PARA INICIAR", 20, 100, 30, DARKBLUE);
	DrawText("R PARA VER O RANKING", 20, 150, 30, DARKBLUE);
    DrawText("ESC PARA SAIR", 20, 200, 30, DARKBLUE);
    
	if (enteringNick) { // Se o jogador estiver digitando o nick
		// Caixa de texto para inserir o nick
        int boxWidth = 300;
        int boxHeight = 80;
        int boxX = screenWidth / 2 - boxWidth / 2;
        int boxY = screenHeight / 2 - boxHeight / 2;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, LIGHTGRAY);  // Caixa para entrada de texto
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, DARKGRAY); // Borda da caixa
        DrawText("Digite seu Nick:", boxX + 10, boxY + 10, 20, DARKBLUE);  // Instrução
        DrawText(nick, boxX + 10, boxY + 40, 30, DARKBLUE);  // Nickname digitado
        DrawText("Pressione ENTER para confirmar", boxX + 10, boxY + 70, 10, GRAY); // Instrução de confirmação
    }

    EndDrawing(); // Finaliza o desenho
}

// ATUALIZAR O JOGO
void UpdateGame(void) {
	// Cria um ponteiro para a lista de obstáculos e um ponteiro para o banhista que aparece na tela
	ListObj *atual = head;
	ListBanhista *banhistaTela = headBanhista;

	if (!gameOver) { // Se o jogo não acabou
		// Movimentação do barco
		if (IsKeyDown(KEY_D)) barco.rec.x += barco.speed.x;
		if (IsKeyDown(KEY_A)) barco.rec.x -= barco.speed.x;
		if (IsKeyDown(KEY_W)) barco.rec.y -= barco.speed.y;
		if (IsKeyDown(KEY_S)) barco.rec.y += barco.speed.y;

		switch (wave) { // Verifica em qual onda de obstáculos o jogador está
            case PRIMEIRA: { 
                if (!smooth) { // Controla o aparecimento do texto da onda
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true; // Se o alpha for maior ou igual a 1, o texto apareceu completamente
                }

                if (smooth) alpha -= 0.02f; // Controla o desaparecimento do texto da onda

                if (score >= 3000) {  // Se o jogador atingir 3000 pontos, passa para a segunda onda
                    activeObstaculos = SEGUNDA_ONDA;
                    wave = SEGUNDA;
                    smooth = false;
                    alpha = 0.0f;

                    for (int i = 0; i < activeObstaculos; i++) { // Re-insere os obstáculos
                        inserirobstaculos(&head, &tail); 
                    }
                }
            } break;
            case SEGUNDA: {
                if (!smooth) { // Controla o aparecimento do texto da onda
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true; // Se o alpha for maior ou igual a 1, o texto apareceu completamente
                }

                if (smooth) alpha -= 0.02f; // Controla o desaparecimento do texto da onda 

                if (score >= 6000) { // Se o jogador atingir 6000 pontos, passa para a terceira onda
                    activeObstaculos = TERCEIRA_ONDA;
                    wave = TERCEIRA;
                    smooth = false;
                    alpha = 0.0f;					

                    for (int i = 0; i < activeObstaculos; i++){ // Re-insere os obstáculos
                        inserirobstaculos(&head, &tail); 
                    }
                }
            } break;
            case TERCEIRA: {
                if (!smooth) { // Controla o aparecimento do texto da onda 
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true; // Se o alpha for maior ou igual a 1, o texto apareceu completamente
                }

                if (smooth) alpha -= 0.02f; // Controla o desaparecimento do texto da onda

                if (score >= 10000) { // Se o jogador atingir 10000 pontos, ele vence o jogo
					victory = true;
					// Atualizar o ranking com o nome do Player e a pontuação
					Player player;
					strcpy(player.nome, nick);
					player.score = score;
					player.banhistas = banhistaSalvos;
					salvarRanking(player.nome, player.score, player.banhistas); // Salva o jogador no ranking
					gameOver = true; // O jogo acaba
					return;
				}
				
            } break;
            default: break;
        }

		timerBanhista += GetFrameTime(); // Incrementa o timer do banhista

		if (timerBanhista >= 5.0f) { // Se o timer do banhista for maior ou igual a 5 segundos, insere um novo banhista
			inserirBanhista(&headBanhista, &tailBanhista);
			timerBanhista = 0; // Reseta o timer
		}

		do { // Percorre a lista de obstáculos e verifica colisões - Lista Circular
			if (atual->obstaculo.active && CheckCollisionRecs(barco.rec, atual->obstaculo.rec)) { // Verifica se houve colisão entre o barco e o obstáculo
				if (score > 0) { // Se a pontuação for maior que 0, decrementa 50 pontos
					score -= 50;
				}
				
				if (banhistaSalvos > 0) { // Se a quantidade de banhistas salvos for maior que 0, decrementa 1 banhista salvo
					banhistaSalvos -= 1;
				}

				if (vida > 0) { // Se a vida for maior que 0, decrementa 1 vida
					vida -= 2;
				}

				removerBanhistaColetados(&headBanhistaColetados, &tailBanhistaColetados); // Remove o primeiro banhista coletado

				atual->obstaculo.active = false; // Desativa o obstáculo

				if (vida == 0) { // Se a vida for igual a 0, o jogo acaba
					timerBanhista = 0; // Reseta o timer do banhista

					// Atualizar o ranking com o nome do Player e a pontuação
					Player player;
					strcpy(player.nome, nick);
					player.score = score;
					player.banhistas = banhistaSalvos;
					salvarRanking(player.nome, player.score, player.banhistas); // Salva o jogador no ranking
					gameOver = true; // O jogo acaba
				}
			}
			
            if (!atual->obstaculo.active) { // Se o obstáculo não estiver ativo
                atual->obstaculo.active = true; // Ativa o obstáculo
                atual->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000); // Posição aleatória na tela em x
                atual->obstaculo.rec.y = GetRandomValue(470, 700); // Posição aleatória na tela em y
            }

            if (atual->obstaculo.active) { // Se o obstáculo estiver ativo
                atual->obstaculo.rec.x -= atual->obstaculo.speed.x; // Movimenta o obstáculo para a esquerda

                if (atual->obstaculo.rec.x < 0) {  // Se o obstáculo sair da tela
					score += 100; // Incrementa 100 pontos
                    atual->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000); // Posição aleatória na tela em x
                    atual->obstaculo.rec.y = GetRandomValue(470, 700); // Posição aleatória na tela em y
                }
            }

            atual = atual->prox; // Atualiza o ponteiro para o próximo obstáculo
        } while (atual != head);

		while (banhistaTela != NULL) { // Percorre a lista de banhistas que aparecem na tela e verifica colisões - Lista Simplesmente Encadeada
			if (banhistaTela->banhista.active && CheckCollisionRecs (barco.rec, banhistaTela->banhista.rec)) { // Verifica se houve colisão entre o barco e o banhista
				
				inserirBanhistaColetados(&headBanhistaColetados, &tailBanhistaColetados); // Insere o banhista coletado
            	banhistaSalvos = countBanhistasColetados(headBanhistaColetados); // Conta a quantidade de banhistas coletados

				if (banhistaSalvos == 50) { // Se a quantidade de banhistas coletados for igual a 50, decrementa a velocidade do barco
					barco.speed.x -= 1;
				} else if (banhistaSalvos < 50) { // Se a quantidade de banhistas coletados for menor que 50, a velocidade do barco é 4
					barco.speed.x = 4;
				}

				if (banhistaSalvos == 100) { // Se a quantidade de banhistas coletados for igual a 100, incrementa a velocidade do barco
					barco.speed.x -= 1;
				} else if (banhistaSalvos < 100 && banhistaSalvos > 50) { // Se a quantidade de banhistas coletados for maior que 50 e menor que 100, a velocidade do barco é 3
					barco.speed.x = 3;
				}

            	banhistaTela->banhista.active = false; // Desativa o banhista
			}

			if (!banhistaTela->banhista.active) { // Se o banhista não estiver ativo
				banhistaTela->banhista.active = true; // Ativa o banhista
				banhistaTela->banhista.rec.x = GetRandomValue(screenWidth, screenWidth + 1000); // Posição aleatória na tela em x
                banhistaTela->banhista.rec.y = GetRandomValue(470, 700); // Posição aleatória na tela em y
			}

			if (banhistaTela->banhista.active) { // Se o banhista estiver ativo
                banhistaTela->banhista.rec.x -= banhistaTela->banhista.speed.x; // Movimenta o banhista para a esquerda

                if (banhistaTela->banhista.rec.x < 0) { // Se o banhista sair da tela
                    atual->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000); // Posição aleatória na tela em x
                    atual->obstaculo.rec.y = GetRandomValue(470, 700); // Posição aleatória na tela em y
                }
            }

			banhistaTela = banhistaTela->prox; // Atualiza o ponteiro para o próximo banhista
		}

		if (barco.rec.x <= 0) { // Limita o movimento do barco em x
			barco.rec.x = 0;
		}

		if (barco.rec.x + barco.rec.width >= screenWidth) { // Impede que o barco suma da tela
			barco.rec.x = screenWidth - barco.rec.width; 
		}

		if (barco.rec.y <= 450) { // Limita o movimento do barco em y
			barco.rec.y = 450;
		}

		if (barco.rec.y + barco.rec.height >= 730) { // Impede que o barco suma da tela
			barco.rec.y = 730 - barco.rec.height;
		}
	} else { // Se o jogo acabou
		if (IsKeyPressed(KEY_ENTER)) { // Se o jogador pressionar ENTER
			InitGame(); // Reinicia o jogo
			gameOver = false; // O jogo não acabou
		} else if (IsKeyPressed(KEY_M)){ // Se o jogador pressionar M
			currentGameState=MENU; // Volta para o menu
			InitGame(); // Reinicia o jogo
		} else if (IsKeyPressed(KEY_R)) { // Se o jogador pressionar R 
			currentGameState=GAME_OVER; // Troca o estado para GAME_OVER
			InitGame(); // Reinicia o jogo
		}
	}
}

// DESENHA O JOGO
void DrawGame(void) {
    BeginDrawing(); // Inicia o desenho

    if (victory) { // Se o jogador vencer o jogo
		// Desenha a imagem de fundo da vitória
        DrawTexture(FundoVitoria, 0, -20, WHITE);
        DrawText("VOCÊ CHEGOU A PETROLANDIA", screenWidth / 2 - MeasureText("VOCÊ CHEGOU A PETROLANDIA", 40) / 2, screenHeight / 2 - 20, 40, BLACK);
		DrawText("M PARA VOLTAR AO MENU", 70, 600, 20, RED);
		DrawText("ESC PARA SAIR", 70, 650, 20, RED);
    } else if (!gameOver) {
        // Desenhar o cenário de fundo
        DrawTexture(CenarioTexture, 0, 0, WHITE);
        
        // Desenhar o barco
        DrawTexture(Barcotextura, (int)barco.rec.x, (int)barco.rec.y, WHITE);

        // Desenhar o texto da onda atual - sendo a cor da onda de acordo com alpha
        if (wave == PRIMEIRA) {
            DrawText("PRIMEIRA ONDA", screenWidth/2 - MeasureText("PRIMEIRA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
		} else if (wave == SEGUNDA) {
            DrawText("SEGUNDA ONDA", screenWidth/2 - MeasureText("SEGUNDA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
		} else if (wave == TERCEIRA) {
            DrawText("TERCEIRA ONDA", screenWidth/2 - MeasureText("TERCEIRA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
		}

        // Desenhar obstáculos
        ListObj *atual = head;

        do { // Lista Circular
            if (atual->obstaculo.active) { // Se o obstáculo estiver ativo
                DrawTexture(PedraTextura, (int)atual->obstaculo.rec.x, (int)atual->obstaculo.rec.y, WHITE);
            }

            atual = atual->prox; // Atualiza o ponteiro para o próximo obstáculo
        } while (atual != head);

        // Desenhar banhistas
        ListBanhista *banhistaTela = headBanhista;

        while (banhistaTela != NULL) { // Lista Simplesmente Encadeada
            if (banhistaTela->banhista.active) { // Se o banhista estiver ativo
                DrawTexture(BanhistaTextura, (int)banhistaTela->banhista.rec.x, (int)banhistaTela->banhista.rec.y, WHITE);
            }

            banhistaTela = banhistaTela->prox; // Atualiza o ponteiro para o próximo banhista
        }

        // Mostrar o nome e score do Player
        int nickPosX = screenWidth / 2 - MeasureText(nick, 30) / 2;
        DrawText(nick, nickPosX, 20, 30, BLUE);

		DrawTexture(moeda, -5, 15, WHITE); // Desenha a moeda
        DrawText(TextFormat("%04i", score), 60, 20, 40, RED); // Mostra a pontuação
		
		DrawTexture(coracaoTextura, 5, 70, WHITE); // Desenha o coração
		DrawText(TextFormat("%03i", vida), 60, 75, 40, RED); // Mostra a vida

        // Mostrar número de banhistas salvos
        DrawTexture(BanhistaTextura, 820, 8, WHITE); // Desenha o banhista
        DrawText(TextFormat("%04i", banhistaSalvos), 900, 20, 40, RED); // Mostra a quantidade de banhistas salvos
    } else {
        // Se o jogo acabou
        DrawGameOverScreen(); // Desenha a tela de game over
    }

    EndDrawing(); // Finaliza o desenho
}

// DESENHA A TELA DE GAME OVER
void salvarRanking(const char* nome, int score, int numBanhistas) {
    Player ranking[MAX_RANKING]; // Crio um vetor de Players
    int count = 0; // Contador de Players que estão no arquivo ranking.txt

    // Abrir ranking.txt em modo de adição e escrever o novo jogador - não sobrescreve o arquivo
    FILE* file = fopen("ranking.txt", "a");

    if (file != NULL) { // Verifica se o arquivo abriu corretamente
        fprintf(file, "%s;%d;%d\n", nome, score, numBanhistas); // Escreve o novo jogador no arquivo
        fclose(file); // Fecha o arquivo após a adição
    }

	// Abrir ranking.txt em modo de leitura
    file = fopen("ranking.txt", "r");

    if (file != NULL) { // Verifica se o arquivo abriu corretamente
        while (fscanf(file, "%[^;];%d;%d\n", ranking[count].nome, &ranking[count].score, &ranking[count].banhistas) != EOF && count < MAX_RANKING) {
            count++; // Conta o número de Players no arquivo
        }

        fclose(file); // Fecha o arquivo após a leitura
    }

    // Ordenar o ranking pelo score
    ordenarRanking(ranking, count);

	// Abrir rankingOrdenado.txt em modo de escrita e escrever os 5 primeiros jogadores - sobrescreve o arquivo
    file = fopen("rankingOrdenado.txt", "w");

    for (int i = 0; i < count && i < 5; i++) { // Escreve os 5 primeiros jogadores no arquivo
        fprintf(file, "%s;%d;%d\n", ranking[i].nome, ranking[i].score, ranking[i].banhistas);
    }

    fclose(file); // Fecha o arquivo após a escrita
}

// ORDENAR RANKING
void ordenarRanking(Player ranking[], int count) {
	// Baseado no algoritmo de ordenação Bubble Sort
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (ranking[j].score < ranking[j + 1].score || 
                (ranking[j].score == ranking[j + 1].score && ranking[j].banhistas < ranking[j + 1].banhistas)) {
                // Troca os elementos
                Player temp = ranking[j];
                ranking[j] = ranking[j + 1];
                ranking[j + 1] = temp;
            }
        }
    }
}

// REMOVER BANHISTA COLETADOS
void removerBanhistaColetados (BanhistaColetados **head, BanhistaColetados **tail) {
	// Libera a memória do primeiro banhista coletado
	if (*head != NULL) {
		BanhistaColetados *aux = *head;
		*head = (*head)->prox;
		
		if (*head == NULL) {
			*tail = NULL;
		}

		free(aux); // Libera a memória do banhista coletado
	}
}

// INSERIR BANHISTA COLETADOS
void inserirBanhistaColetados(BanhistaColetados **head, BanhistaColetados **tail) {

    BanhistaColetados *novo = (BanhistaColetados *)malloc(sizeof(BanhistaColetados)); // Aloca memória para um novo banhista coletado

    if (novo != NULL) {
		// Características do banhista coletado
        novo->banhista.rec.width = 15;
        novo->banhista.rec.height = 15;
        novo->banhista.active = true;
        novo->prox = NULL;
		
        if (*head == NULL) {
            *head = novo;
            *tail = novo;
        } else {
            (*tail)->prox = novo;
            *tail = novo;
        }
    }
}

// CONTAR BANHISTAS COLETADOS
int countBanhistasColetados(BanhistaColetados *head) {
	// Conta a quantidade de banhistas coletados
    BanhistaColetados *atual = head;
    int count = 0;
	
    while (atual != NULL) { // Percorre a lista contando os banhistas coletados
        count++; // Incrementa o contador
        atual = atual->prox;
    }

    return count;
}

// DESENHA A TELA DE GAME OVER
void DrawGameOverScreen(void) {
	// Abre o arquivo rankingOrdenado.txt em modo de leitura
    FILE *file = fopen("rankingOrdenado.txt", "r");

    Player ranking[MAX_RANKING]; // Crio um vetor de Players
    
    // Verifica se o arquivo abriu corretamente
	if (file != NULL) { 
		char line[100]; // Cria um vetor de char para armazenar a linha do arquivo

		for (int i = 0; i < MAX_RANKING && fgets(line, sizeof(line), file); i++) { // Realiza o "split" da linha por ";"

			char *token = strtok(line, ";"); // Separa a linha por ";"
			if (token != NULL) { 
				strcpy(ranking[i].nome, token);  // Copia o nome do Player para o vetor de Players
			}
			
			token = strtok(NULL, ";"); // Atualiza o token
			if (token != NULL) { 
				ranking[i].score = atoi(token); // Converte a pontuação do Player para inteiro e armazena no vetor de Players
			}
			
			token = strtok(NULL, ";"); // Atualiza o token
			if (token != NULL) { 
				ranking[i].banhistas = atoi(token); // Converte a quantidade de banhistas salvos do Player para inteiro e armazena no vetor de Players
			}
		} 
		fclose(file); // Fechar o arquivo após a leitura 
	} else { // Tratar o erro de abertura do arquivo, se necessário 
		printf("Erro ao abrir o arquivo de ranking.\n"); 
	}

    BeginDrawing(); // Inicia o desenho

    ClearBackground(BLUE);

	DrawRectangle(50, 50, screenWidth -100, screenHeight -100, LIGHTGRAY);
	DrawRectangleLines(50, 50, screenWidth -100, screenHeight -100, DARKGRAY);

    // Exibir as informações do ranking
    for (int i = 0; i < 5; i++) {
		DrawText(ranking[i].nome, 70, 80 + 30 * i, 20, DARKBLUE);
        DrawText(TextFormat("Score: %d", ranking[i].score), 200, 80 + 30 * i, 20, DARKGREEN); 
		DrawText(TextFormat("Banhistas: %d", ranking[i].banhistas), 350, 80 + 30 * i, 20, DARKPURPLE);

		DrawText("Ranking", screenWidth / 2 - MeasureText("Ranking", 40) / 2 + 2, screenHeight / 2 - 100 + 2, 40, GRAY); 
		DrawText("Ranking", screenWidth / 2 - MeasureText("Ranking", 40) / 2, screenHeight / 2 - 100, 40, RED);     
	}	
	DrawText("M PARA VOLTAR AO MENU", 70, 600, 20, DARKBLUE);
	DrawText("ESC PARA SAIR", 70, 650, 20, DARKBLUE);

	EndDrawing(); // Finaliza o desenho
}

// DESCARREGAR AS TEXTURAS
void UnloadGame(void) {
	UnloadTexture(Barcotextura);
	UnloadTexture(CenarioTexture);
	UnloadTexture(MenuInicial);
	UnloadTexture(PedraTextura);
	UnloadTexture(BanhistaTextura);
	UnloadTexture(FundoVitoria);
	UnloadTexture(coracaoTextura);
	UnloadTexture (moeda);
}