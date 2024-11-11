#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_WEB)
	#include <emscripten/emscripten.h>
#endif

#define NUM_MAX_OBJETOS 13
#define PRIMEIRA_ONDA 7
#define SEGUNDA_ONDA 10
#define TERCEIRA_ONDA 13

#define NUM_MAX_BANHISTA 5
#define MAX_RANKING 5

typedef enum { PRIMEIRA = 0, SEGUNDA, TERCEIRA } OndaObstaculo;
typedef enum { MENU = 0, PLAY, GAME_OVER } GameState;

typedef struct Barco{
	Rectangle rec;
	Vector2 speed;
} Barco;

typedef struct Obstaculo{
	Rectangle rec;
	Vector2 speed;
	bool active;
	Color cor;
} Obstaculo;

typedef struct ListObj{
	Obstaculo obstaculo;
	struct ListObj *prox;
} ListObj;

typedef struct Banhista{
	Rectangle rec;
	Vector2 speed;
	bool active;
	Color cor;
} Banhista;

typedef struct ListBanhista{
	Banhista banhista;
	struct Listbanhista *prox;
} ListBanhista;

typedef struct BanhistaColetados{
	Banhista banhista;
	struct Listbanhista *prox;
} BanhistaColetados;

typedef struct Player {
		char nome[50];
		int score;
		int banhistas;
} Player;

Obstaculo *head = NULL;
Obstaculo *tail = NULL;

Banhista *headBanhista = NULL;
Banhista *tailBanhista = NULL;

BanhistaColetados *headBanhistaColetados = NULL;
BanhistaColetados *tailBanhistaColetados = NULL;

static const int screenWidth = 1000;
static const int screenHeight = 749;

static GameState currentGameState = MENU;
static char nick[20] = {0};
static int nickIndex = 0;
static bool enteringNick = false;

static bool gameOver = false;
static int score = 0;
static int banhistaSalvos = 0;
static float timerBanhista = 0;
static bool victory = false;
static int vida = 100;

static Barco barco = { 0 };
static Obstaculo obstaculo[NUM_MAX_OBJETOS] = { 0 };
static Banhista banhista[NUM_MAX_BANHISTA] = { 0 };
static OndaObstaculo wave = { 0 };

static float alpha = 0.0f;

static int activeObstaculos = 0;
static bool smooth = false;

Texture2D Barcotextura;
Texture2D CenarioTexture;
Texture2D MenuInicial;
Texture2D PedraTextura;
Texture2D BanhistaTextura;
Texture2D FundoVitoria;
Texture2D coracaoTextura;
Texture2D moeda;

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);
static void DrawGameOverScreen(void);

int main(void){
	InitWindow(screenWidth, screenHeight, "Petrolandia Adventure");
	InitGame();
	#if defined(PLATFORM_WEB)
		emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
	#else
		SetTargetFPS(60);
		while (!WindowShouldClose()){
			UpdateDrawFrame();
		}
	#endif
		UnloadGame();
		CloseWindow();
		printf("Score: %d\n", score);
		printf("Banhistas Salvos: %d", banhistaSalvos);
		printf("Vida: %d", vida);
		return 0;
}

void InitGame(void){
	limpaObstaculos(&head, &tail);
	limparBanhistas(&headBanhista, &tailBanhista);
	limparBanhistasColetados(&headBanhistaColetados, &tailBanhistaColetados);
	
	Barcotextura = LoadTexture("resources/Barco.png");
	CenarioTexture = LoadTexture("resources/Cenario.png");
	MenuInicial = LoadTexture("resources/MenuInical.png");
	PedraTextura = LoadTexture("resources/pedra.png");
	BanhistaTextura = LoadTexture("resources/Pessoa4.png");
	FundoVitoria = LoadTexture("resources/fim.png");
	coracaoTextura = LoadTexture("resources/Coracao.png");
	moeda = LoadTexture ("resources/moeda.png");

	gameOver = false;
	victory = false;
	smooth = false;
	wave = PRIMEIRA;
	activeObstaculos = PRIMEIRA_ONDA;
	score = 0;
	vida = 100;
	alpha = 0;

	barco.rec.x = 20;
	barco.rec.y = 50;
	barco.rec.width = Barcotextura.width;
	barco.rec.height = Barcotextura.height - 30;
	barco.speed.x = 4;
	barco.speed.y = 4;

	for (int i = 0; i < activeObstaculos; i++){
		inserirobstaculos(&head, &tail);
	}
	for (int i = 0; i <NUM_MAX_BANHISTA; i++){
		inserirBanhista(&headBanhista, &tailBanhista);
	}
}

void inserirobstaculos (ListObj **head, ListObj **tail) {
  ListObj *novo = (ListObj *) malloc(sizeof(ListObj));
  if(novo != NULL){
    novo->obstaculo.rec.width = PedraTextura.width - 45;
    novo->obstaculo.rec.height = PedraTextura.height - 45;
    novo->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
    novo->obstaculo.rec.y = GetRandomValue(470, 700);
	novo->obstaculo.speed.x = 5;
	novo->obstaculo.speed.y = 5;
	novo->obstaculo.active = true;
	novo->obstaculo.cor = DARKGRAY;
	novo->prox = *head;
    if(*head==NULL){
		*head = *tail = novo;
		(*tail)->prox = novo;
    }else{
		(*tail)->prox = novo;
		novo->prox = *head;
		*tail = novo;
    }
  }
}

void limpaObstaculos(ListObj **head, ListObj **tail) {
    if (*head == NULL){
		return;
	}
	else{
		ListObj *aux = *head;
    	ListObj *prox;
    	do {
        	prox = aux->prox;
        	free(aux);
        	aux = prox;
    	} while (aux != *head);
		*head = NULL;
		*tail = NULL;
	}
}

void inserirBanhista (ListBanhista **head, ListBanhista **tail){
    ListBanhista *nova = (ListBanhista *)malloc(sizeof(ListBanhista));
    if (nova != NULL){
        nova->banhista.rec.width = 15;
        nova->banhista.rec.height = 15;
        nova->banhista.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
        nova->banhista.rec.y = GetRandomValue(470, 700);
        nova->banhista.speed.x = 5;
        nova->banhista.speed.y = 5;
        nova->banhista.active = true;
        nova->banhista.cor = RED;
        nova->prox = NULL;
        if (*head == NULL){
            *head = nova;
            *tail = nova;
        }
		else {
            (*tail)->prox = nova;
            *tail = nova;
        }
    }
}

void limparBanhistas(ListBanhista **head, ListBanhista **tail){
	if (*head == NULL){
		return;
	}
	else{
		ListBanhista *aux = *head;
		ListBanhista *prox = *head;
		while (aux != NULL){
			prox = prox->prox;
			free(aux);
			aux = prox;
		}
		*head = NULL;
		*tail = NULL;
	}
}

void inserirBanhistaColetados(BanhistaColetados **head, BanhistaColetados **tail) {
    BanhistaColetados *novo = (BanhistaColetados *)malloc(sizeof(BanhistaColetados));
    if (novo != NULL) {
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

int countBanhistasColetados(BanhistaColetados *head) {
    int count = 0;
    BanhistaColetados *atual = head;
    while (atual != NULL) {
        count++;
        atual = atual->prox;
    }
    return count;
}

void limparBanhistasColetados(BanhistaColetados **head, BanhistaColetados **tail){
	if (*head == NULL){
		return;
	}
	else{
		BanhistaColetados *aux = *head;
		BanhistaColetados *prox = *head;
		while (aux != NULL){
			prox = prox->prox;
			free(aux);
			aux = prox;
		}
		*head = NULL;
		*tail = NULL;
	}
}

void removerBanhistaColetados (BanhistaColetados **head, BanhistaColetados **tail){
	if (*head != NULL){
		BanhistaColetados *aux = *head;
		*head = (*head)->prox;
		if (*head == NULL){
			*tail = NULL;
		}
		free(aux);
	}
}

void DrawMenu(void) {
    BeginDrawing();
    
    DrawTexture(MenuInicial, 0, 0, WHITE);
    
    DrawText("PETROLÂNDIA ADVENTURE", 20, 20, 45, DARKBLUE);
    
    DrawText("ENTER PARA INICIAR", 20, 100, 30, DARKBLUE);
	DrawText("R PARA VER O RANKING", 20, 150, 30, DARKBLUE);
    DrawText("ESC PARA SAIR", 20, 200, 30, DARKBLUE);
    
	if (enteringNick) {
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

    EndDrawing();
} 

void UpdateMenu(void) {
    if (IsKeyPressed(KEY_ENTER)) {
        if (!enteringNick) {
            enteringNick = true; // Inicia a entrada do nick
        } else if (nickIndex > 0) { // Confirma o nick e inicia o jogo se já tiver digitado algo
            enteringNick = false;
            currentGameState = PLAY;
        }
    }

    if (enteringNick) {
        int key = GetKeyPressed();

        // Captura de texto
        if (key >= 32 && key <= 125 && nickIndex < 19) { // Letras, números e símbolos
            nick[nickIndex++] = (char)key;
            nick[nickIndex] = '\0';
        }

        // Apagar último caractere
        if (IsKeyPressed(KEY_BACKSPACE) && nickIndex > 0) {
            nick[--nickIndex] = '\0';
        }
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
    }else if (IsKeyPressed(KEY_R)) {
        currentGameState = GAME_OVER;
    }
}

void UpdateGame(void){
	ListObj *atual = head;
	ListBanhista *banhistaTela = headBanhista;

	if (!gameOver){

		if (IsKeyDown(KEY_D)) barco.rec.x += barco.speed.x;
		if (IsKeyDown(KEY_A)) barco.rec.x -= barco.speed.x;
		if (IsKeyDown(KEY_W)) barco.rec.y -= barco.speed.y;
		if (IsKeyDown(KEY_S)) barco.rec.y += barco.speed.y;

		switch (wave){
            case PRIMEIRA:{
                if (!smooth){
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true;
                }

                if (smooth) alpha -= 0.02f;

                if (score >= 3000){
                    activeObstaculos = SEGUNDA_ONDA;
                    wave = SEGUNDA;
                    smooth = false;
                    alpha = 0.0f;
                    for (int i = 0; i < activeObstaculos; i++){
                        inserirobstaculos(&head, &tail);
                    }
                }
            } break;
            case SEGUNDA:{
                if (!smooth){
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true;
                }

                if (smooth) alpha -= 0.02f;

                if (score >= 6000){
                    activeObstaculos = TERCEIRA_ONDA;
                    wave = TERCEIRA;
                    smooth = false;
                    alpha = 0.0f;					

                    for (int i = 0; i < activeObstaculos; i++){
                        inserirobstaculos(&head, &tail);
                    }
                }
            } break;
            case TERCEIRA:{
                if (!smooth){
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true;
                }
                if (smooth) alpha -= 0.02f;

                if (score >= 10000){
					victory = true;
					gameOver = true;
					return;
				}
				
            } break;
            default: break;
        }

		timerBanhista += GetFrameTime();
		if (timerBanhista >= 5.0f) {
			inserirBanhista(&headBanhista, &tailBanhista);
			timerBanhista = 0;
		}

		atual = head;

		do {
			switch (wave){
				case PRIMEIRA:{
					if (atual->obstaculo.active && CheckCollisionRecs(barco.rec, atual->obstaculo.rec)) {
						if (score > 0){
							score -= 50;
						}
						if (banhistaSalvos > 0){
							banhistaSalvos -= 1;
						}
						if (vida > 0){
							vida -= 1;
						}
						removerBanhistaColetados (&headBanhistaColetados, &tailBanhistaColetados);
						atual->obstaculo.active = false;
						if (vida == 0) {
							timerBanhista = 0;
							gameOver = true;
						}
            		}
				} break;
				case SEGUNDA:{
					if (atual->obstaculo.active && CheckCollisionRecs(barco.rec, atual->obstaculo.rec)) {
						if (score > 0){
							score -= 100;
						}
						if (banhistaSalvos > 0){
							banhistaSalvos -= 1;
						}
						if (vida > 0){
							vida -= 1;
						}
						removerBanhistaColetados (&headBanhistaColetados, &tailBanhistaColetados);
						atual->obstaculo.active = false;
						if (vida == 0) {
							timerBanhista = 0;
							gameOver = true;
						}
            		}
				} break;
				case TERCEIRA:{
					if (atual->obstaculo.active && CheckCollisionRecs(barco.rec, atual->obstaculo.rec)) {
						if (score > 0){
							score -= 150;
						}
						if (banhistaSalvos > 0){
							banhistaSalvos -= 1;
						}
						if (vida > 0){
							vida -= 1;
						}
						removerBanhistaColetados (&headBanhistaColetados, &tailBanhistaColetados);
						atual->obstaculo.active = false;
						if (vida == 0) {
							timerBanhista = 0;
							gameOver = true;
						}
            		}
				} break;
				default: break;
			}

            if (!atual->obstaculo.active) {
                atual->obstaculo.active = true;
                atual->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                atual->obstaculo.rec.y = GetRandomValue(470, 700);
            }

            if (atual->obstaculo.active) {
                atual->obstaculo.rec.x -= atual->obstaculo.speed.x;
                if (atual->obstaculo.rec.x < 0) {
					score += 100;
                    atual->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    atual->obstaculo.rec.y = GetRandomValue(470, 700);
                }
            }

            atual = atual->prox;
        } while (atual != head);

		while (banhistaTela != NULL){
			if (banhistaTela->banhista.active && CheckCollisionRecs (barco.rec, banhistaTela->banhista.rec)){
				inserirBanhistaColetados(&headBanhistaColetados, &tailBanhistaColetados);
            	banhistaSalvos = countBanhistasColetados(headBanhistaColetados);
				if (banhistaSalvos == 50){
					barco.speed.x -= 1;
				}else if (banhistaSalvos < 50){
					barco.speed.x = 4;
				}
				if (banhistaSalvos == 100){
					barco.speed.x -= 1;
				}
				else if (banhistaSalvos < 100 && banhistaSalvos > 50){
					barco.speed.x = 3;
				}
            	banhistaTela->banhista.active = false;
			}
			if (!banhistaTela->banhista.active){
				banhistaTela->banhista.active = true;
				banhistaTela->banhista.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                banhistaTela->banhista.rec.y = GetRandomValue(470, 700);
			}
			if (banhistaTela->banhista.active) {
                banhistaTela->banhista.rec.x -= banhistaTela->banhista.speed.x;
                if (banhistaTela->banhista.rec.x < 0) {
                    atual->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    atual->obstaculo.rec.y = GetRandomValue(470, 700);
                }
            }
			banhistaTela = banhistaTela->prox;
		}

		if (barco.rec.x <= 0){
			barco.rec.x = 0;
		}
		if (barco.rec.x + barco.rec.width >= screenWidth){
			barco.rec.x = screenWidth - barco.rec.width;
		}
		if (barco.rec.y <= 450){
			barco.rec.y = 450;
		}
		if (barco.rec.y + barco.rec.height >= 730){
			barco.rec.y = 730 - barco.rec.height;
		}
	}
	else{
		if (IsKeyPressed(KEY_ENTER)){
			InitGame();
			gameOver = false;
		} else if (IsKeyPressed(KEY_M)){
			currentGameState=MENU;
			InitGame();
		} else if (IsKeyPressed(KEY_R)){
			currentGameState=GAME_OVER;
			InitGame();
		}
	}
}

void DrawGame(void){
    BeginDrawing();

    if (victory){
        DrawTexture(FundoVitoria, 0, -20, WHITE);
        DrawText("VOCÊ CHEGOU A PETROLANDIA", screenWidth / 2 - MeasureText("VOCÊ CHEGOU A PETROLANDIA", 40) / 2, screenHeight / 2 - 20, 40, BLACK);
		DrawText("M PARA VOLTAR AO MENU", 70, 600, 20, RED);
		DrawText("ESC PARA SAIR", 70, 650, 20, RED);
    }
    else if(!gameOver){
        // Desenhar o cenário de fundo
        DrawTexture(CenarioTexture, 0, 0, WHITE);
        
        // Desenhar o barco
        DrawTexture(Barcotextura, (int)barco.rec.x, (int)barco.rec.y, WHITE);

        // Desenhar o texto da onda atual
        if (wave == PRIMEIRA) 
            DrawText("PRIMEIRA ONDA", screenWidth/2 - MeasureText("PRIMEIRA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
        else if (wave == SEGUNDA) 
            DrawText("SEGUNDA ONDA", screenWidth/2 - MeasureText("SEGUNDA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
        else if (wave == TERCEIRA) 
            DrawText("TERCEIRA ONDA", screenWidth/2 - MeasureText("TERCEIRA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));

        // Desenhar obstáculos
        ListObj *atual = head;
        do {
            if (atual->obstaculo.active) {
                DrawTexture(PedraTextura, (int)atual->obstaculo.rec.x, (int)atual->obstaculo.rec.y, WHITE);
            }
            atual = atual->prox;
        } while (atual != head);

        // Desenhar banhistas
        ListBanhista *banhistaTela = headBanhista;
        while (banhistaTela != NULL) {
            if (banhistaTela->banhista.active) {
                DrawTexture(BanhistaTextura, (int)banhistaTela->banhista.rec.x, (int)banhistaTela->banhista.rec.y, WHITE);
            }
            banhistaTela = banhistaTela->prox;
        }

        // Mostrar o nome e score do Player
        int nickPosX = screenWidth / 2 - MeasureText(nick, 30) / 2;
        DrawText(nick, nickPosX, 20, 30, BLUE);

		DrawTexture(moeda, -5, 15, WHITE);
        DrawText(TextFormat("%04i", score), 60, 20, 40, RED);
		
		DrawTexture(coracaoTextura, 5, 70, WHITE);
		DrawText(TextFormat("%03i", vida), 60, 75, 40, RED);

        // Mostrar número de banhistas salvos
        DrawTexture(BanhistaTextura, 820, 8, WHITE);
        DrawText(TextFormat("%04i", banhistaSalvos), 900, 20, 40, RED);

    } else {
        // Se o jogo acabou
        DrawGameOverScreen();
    }

    EndDrawing();
}

void UnloadGame(void){
	UnloadTexture(Barcotextura);
	UnloadTexture(CenarioTexture);
	UnloadTexture(MenuInicial);
	UnloadTexture(PedraTextura);
	UnloadTexture(BanhistaTextura);
	UnloadTexture(FundoVitoria);
	UnloadTexture(coracaoTextura);
	UnloadTexture (moeda);
}

void UpdateDrawFrame(void) {
    switch (currentGameState) {
        case MENU:
            UpdateMenu();
            DrawMenu();
            break;
        case PLAY:
            UpdateGame();
            DrawGame();
            break;
		case GAME_OVER:
			DrawGameOverScreen();
			if (IsKeyPressed(KEY_ENTER)) {
				currentGameState = PLAY;
			} else if (IsKeyPressed(KEY_M)) {
				currentGameState = MENU;
			}
            break;
        default:
            break;
    }
}

void DrawGameOverScreen(void) {
    FILE *file = fopen("ranking.txt", "r");
    Player ranking[MAX_RANKING];
    
    // Inicializar o ranking com valores padrão
    for (int i = 0; i < MAX_RANKING; i++) {
        strcpy(ranking[i].nome, "Vazio");
        ranking[i].score = 0;
        ranking[i].banhistas = 0;
    }
    
    // Verifica se o arquivo abriu corretamente
	if (file != NULL) { 
	char line[100]; 
	for (int i = 0; i < MAX_RANKING && fgets(line, sizeof(line), file); i++) { // Realiza o split da linha por ";" 
		char *token = strtok(line, ";"); 
		if (token != NULL) { 
			strcpy(ranking[i].nome, token); 
		} token = strtok(NULL, ";"); 
		if (token != NULL) { 
			ranking[i].score = atoi(token); 
		} token = strtok(NULL, ";"); 
		if (token != NULL) { 
			ranking[i].banhistas = atoi(token); 
		} 
	} 
	fclose(file); // Fechar o arquivo após a leitura 
	} else { // Tratar o erro de abertura do arquivo, se necessário 
	printf("Erro ao abrir o arquivo de ranking.\n"); 
	}

    BeginDrawing();
    ClearBackground(BLUE);

	DrawRectangle(50, 50, screenWidth -100, screenHeight -100, LIGHTGRAY);
	DrawRectangleLines(50, 50, screenWidth -100, screenHeight -100, DARKGRAY);

    // Exibir as informações do ranking
    for (int i = 0; i < MAX_RANKING; i++) {
		DrawText(ranking[i].nome, 70, 80 + 30 * i, 20, DARKBLUE);
        DrawText(TextFormat("Score: %d", ranking[i].score), 200, 80 + 30 * i, 20, DARKGREEN); 
		DrawText(TextFormat("Banhistas: %d", ranking[i].banhistas), 350, 80 + 30 * i, 20, DARKPURPLE);

		DrawText("Game Over", screenWidth / 2 - MeasureText("Game Over", 40) / 2 + 2, screenHeight / 2 - 100 + 2, 40, GRAY); 
		DrawText("Game Over", screenWidth / 2 - MeasureText("Game Over", 40) / 2, screenHeight / 2 - 100, 40, RED);     
	}	
	DrawText("M PARA VOLTAR AO MENU", 70, 600, 20, DARKBLUE);
	DrawText("ESC PARA SAIR", 70, 650, 20, DARKBLUE);

	EndDrawing();
}

void salvarRanking(const char* nome, int score, int numBanhistas) {
    Player ranking[MAX_RANKING + 1];
    int count = 0;

    // Ler o ranking atual do arquivo
    FILE* file = fopen("ranking.txt", "r");
    if (file != NULL) {
        while (fscanf(file, "%s %d %d", ranking[count].nome, &ranking[count].score, &ranking[count].numBanhistas) != EOF && count < MAX_RANKING) {
            count++;
        }
        fclose(file);
    }

    // Adicionar o novo Player
    strcpy(ranking[count].nome, nome);
    ranking[count].score = score;
    ranking[count].numBanhistas = numBanhistas;
    count++;

    // Ordenar o ranking
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (ranking[j].score > ranking[i].score || 
                (ranking[j].score == ranking[i].score && ranking[j].numBanhistas > ranking[i].numBanhistas)) {
                Player temp = ranking[i];
                ranking[i] = ranking[j];
                ranking[j] = temp;
            }
        }
    }

    // Salvar o ranking atualizado no arquivo
    file = fopen("ranking.txt", "w");
    for (int i = 0; i < count && i < MAX_RANKING; i++) {
        fprintf(file, "%s %d %d\n", ranking[i].nome, ranking[i].score, ranking[i].numBanhistas);
    }
    fclose(file);
}