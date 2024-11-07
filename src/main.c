#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(PLATFORM_WEB)
	#include <emscripten/emscripten.h>
#endif

#define NUM_MAX_ENEMIES 20
#define FIRST_WAVE 7
#define SECOND_WAVE 15
#define THIRD_WAVE 20

#define NUM_MAX_BANHISTA 5

typedef enum { FIRST = 0, SECOND, THIRD } OndaObstaculo;
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

Obstaculo *head = NULL;
Obstaculo *tail = NULL;

Banhista *headBanhista = NULL;
Banhista *tailBanhista = NULL;

BanhistaColetados *headBanhistaColetados = NULL;
BanhistaColetados *tailBanhistaColetados = NULL;

static const int screenWidth = 1000;
static const int screenHeight = 749;

static GameState currentGameState = MENU;
static char nick[20] = {0};  // Nickname do jogador
static int nickIndex = 0;     // Índice de entrada do nick
static bool enteringNick = false;

static bool gameOver = false;
static int score = 0;
static int banhistaSalvos = 0;
static float timerBanhista = 0;
static bool victory = false;

static Barco barco = { 0 };
static Obstaculo obstaculo[NUM_MAX_ENEMIES] = { 0 };
static Banhista banhista[NUM_MAX_BANHISTA] = { 0 };
static OndaObstaculo wave = { 0 };

static float alpha = 0.0f;

static int activeEnemies = 0;
static int enemiesKill = 0;
static bool smooth = false;

Texture2D Barcotextura;
Texture2D CenarioTexture;
Texture2D MenuInicial;

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);

int main(void){
	InitWindow(screenWidth, screenHeight, "classic game: space invaders");
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

		return 0;
}

void InitGame(void){
	limpaObstaculos(&head, &tail);
	limparBanhistas(&headBanhista, &tailBanhista);

	Barcotextura = LoadTexture("resources/Barco.png");
	CenarioTexture = LoadTexture("resources/Cenario.png");
	MenuInicial = LoadTexture("resources/MenuInical.png");

	gameOver = false;
	victory = false;
	smooth = false;
	wave = FIRST;
	activeEnemies = FIRST_WAVE;
	score = 0;
	alpha = 0;

	barco.rec.x = 20;
	barco.rec.y = 50;
	barco.rec.width = Barcotextura.width;
	barco.rec.height = Barcotextura.height - 30;
	barco.speed.x = 4;
	barco.speed.y = 4;

	for (int i = 0; i < 7; i++){
		inserirobstaculos(&head, &tail);
	}
	for (int i = 0; i <NUM_MAX_BANHISTA; i++){
		inserirBanhista(&headBanhista, &tailBanhista);
	}
}

void inserirobstaculos (ListObj **head, ListObj **tail) {
  ListObj *novo = (ListObj *) malloc(sizeof(ListObj));
  if(novo != NULL){
    novo->obstaculo.rec.width = 15;
    novo->obstaculo.rec.height = 15;
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

void DrawMenu(void) {
    BeginDrawing();
    
    DrawTexture(MenuInicial, 0, 0, WHITE);
    
    DrawText("PETROLÂNDIA ADVENTURE", 20, 20, 45, DARKBLUE);
    
    DrawText("ENTER PARA INICIAR", 20, 100, 30, DARKBLUE);
    DrawText("ESC PARA SAIR", 20, 150, 30, DARKBLUE);
    
	if (enteringNick) {
        int boxWidth = 300;
        int boxHeight = 80;
        int boxX = screenWidth / 2 - boxWidth / 2;
        int boxY = screenHeight / 2 - boxHeight / 2;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, LIGHTGRAY);  // Caixa para entrada de texto
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, DARKGRAY); // Borda da caixa
        DrawText("Digite seu Nick:", boxX + 10, boxY + 10, 20, DARKBLUE);  // Instrução
        DrawText(nick, boxX + 10, boxY + 40, 30, DARKBLUE);  // Nickname digitado

        DrawText("Pressione ENTER para confirmar", boxX + 10, boxY + 60, 10, GRAY); // Instrução de confirmação
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
    }
}

void UpdateGame(void){
	ListObj *atual = head;
	ListBanhista *banhistaTela = headBanhista;

	if (!gameOver){
		if (!smooth){
			alpha += 0.02f;
			if (alpha >= 1.0f) smooth = true;
		}
		if (smooth) alpha -= 0.02f;

		if (enemiesKill == activeEnemies){
			enemiesKill = 0;
			do{
				if (!atual->obstaculo.active) {
                	atual->obstaculo.active = true;
				}
				atual = atual->prox;
			} while (atual != head);

			activeEnemies = SECOND_WAVE;
			wave = SECOND;
			smooth = false;
			alpha = 0.0f;
		}

		timerBanhista += GetFrameTime();
		if (timerBanhista >= 5.0f) {
			inserirBanhista(&headBanhista, &tailBanhista);
			timerBanhista = 0;
	}

		atual = head;

		if (IsKeyDown(KEY_D)) barco.rec.x += barco.speed.x;
		if (IsKeyDown(KEY_A)) barco.rec.x -= barco.speed.x;
		if (IsKeyDown(KEY_W)) barco.rec.y -= barco.speed.y;
		if (IsKeyDown(KEY_S)) barco.rec.y += barco.speed.y;

		do {
            if (atual->obstaculo.active && CheckCollisionRecs(barco.rec, atual->obstaculo.rec)) {
                score -= 50;
				banhistaSalvos -= 1;
                atual->obstaculo.active = false;
                if (score == 0) {
					timerBanhista = 0;
                    gameOver = true;
                }
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
				}
				if (banhistaSalvos == 100){
					barco.speed.x -= 1;
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
		}
	}
}

void DrawGame(void){
	BeginDrawing();
	DrawTexture(CenarioTexture, 0, 0, WHITE);

	if (!gameOver){
		DrawTexture(Barcotextura, (int)barco.rec.x, (int)barco.rec.y, WHITE);

		if (wave == FIRST) DrawText("INICIO DA AVENTURA", screenWidth/2 - MeasureText("INICIO DA AVENTURA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));

		ListObj *atual = head;
		do{
			if (atual->obstaculo.active){
				DrawRectangleRec(atual->obstaculo.rec, atual->obstaculo.cor);
			}
			atual = atual->prox;
		}while (atual != head);

		ListBanhista *banhistaTela = headBanhista;
        while (banhistaTela != NULL){
            DrawRectangleRec(banhistaTela->banhista.rec, banhistaTela->banhista.cor);
            banhistaTela = banhistaTela->prox;
        }
		 int nickPosX = screenWidth / 2 - MeasureText(nick, 30) / 2; // Centraliza horizontalmente
    	DrawText(nick, nickPosX, 20, 30, BLUE);
		DrawText(TextFormat("%04i", score), 20, 20, 40, RED);
		DrawText(TextFormat("%04i", banhistaSalvos), 900, 20, 40, RED);

		if (victory) DrawText("YOU WIN", screenWidth/2 - MeasureText("YOU WIN", 40)/2, screenHeight/2 - 40, 40, BLACK);
	}
	else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, BLACK);

	EndDrawing();
}

void UnloadGame(void){
	UnloadTexture(Barcotextura);
	UnloadTexture(CenarioTexture);
	UnloadTexture(MenuInicial);
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
            DrawGame();
            if (IsKeyPressed(KEY_ENTER)) {
                currentGameState = MENU;
            }
            break;
        default:
            break;
    }
}