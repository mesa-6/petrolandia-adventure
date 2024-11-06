#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(PLATFORM_WEB)
	#include <emscripten/emscripten.h>
#endif

//A onda seria por pontuação ( Onda 1 -  )
//A jeitar geração de obstáculos por onda
#define NUM_MAX_ENEMIES 20
#define FIRST_WAVE 7
#define SECOND_WAVE 15
#define THIRD_WAVE 20

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

Obstaculo *head = NULL;
Obstaculo *tail = NULL;

static const int screenWidth = 1000;
static const int screenHeight = 749;

static GameState currentGameState = MENU;

static bool gameOver = false;
static int score = 0;
static bool victory = false;

static Barco barco = { 0 };
static Obstaculo obstaculo[NUM_MAX_ENEMIES] = { 0 };
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

// Initialize game variables
void InitGame(void){
	limpaObstaculos(&head, &tail);
	Barcotextura = LoadTexture("resources/Barco.png");
	CenarioTexture = LoadTexture("resources/Cenario.png");
	MenuInicial = LoadTexture("resources/MenuInical.png");

	gameOver = false;
	victory = false;
	smooth = false;
	wave = FIRST;
	activeEnemies = FIRST_WAVE;
	//Pontuação quando chega no final da tela ( 10 em 10 )
	score = 0;
	alpha = 0;

	// Initialize player
	barco.rec.x = 20;
	barco.rec.y = 50;
	barco.rec.width = Barcotextura.width;
	barco.rec.height = Barcotextura.height - 30;
	barco.speed.x = 4;
	barco.speed.y = 4;

	// Initialize enemies
	for (int i = 0; i < 7; i++){
		inserirobstaculos(&head, &tail);
	}
}

//insere um novo obstaculo na lista
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

// Desenhando o Menu
void DrawMenu(void) {
    BeginDrawing();
    
    DrawTexture(MenuInicial, 0, 0, WHITE);
    
    // Título do Jogo (Posicionado no canto superior esquerdo)
    DrawText("PETROLÂNDIA ADVENTURE", 20, 20, 45, DARKBLUE);
    
    // Texto de instrução (Posicionado abaixo do título)
    DrawText("ENTER PARA INICIAR", 20, 100, 30, DARKBLUE);
    DrawText("ESC PARA SAIR", 20, 150, 30, DARKBLUE);
    
    EndDrawing();
} 

void UpdateMenu(void) {
    if (IsKeyPressed(KEY_ENTER)) {
        currentGameState = PLAY;
        InitGame();
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
    }
}

void UpdateGame(void){
	ListObj *atual = head;

	if (!gameOver){
		//Mostra na tela "INICIO DA AVENTURA"
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

		atual = head;

		//Movimentação
		if (IsKeyDown(KEY_D)) barco.rec.x += barco.speed.x;
		if (IsKeyDown(KEY_A)) barco.rec.x -= barco.speed.x;
		if (IsKeyDown(KEY_W)) barco.rec.y -= barco.speed.y;
		if (IsKeyDown(KEY_S)) barco.rec.y += barco.speed.y;

		//Colisão
		do {
            if (atual->obstaculo.active && CheckCollisionRecs(barco.rec, atual->obstaculo.rec)) {
                score -= 10;
                atual->obstaculo.active = false;
                if (score == 0) {
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
					score += 10;
                    atual->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    atual->obstaculo.rec.y = GetRandomValue(470, 700);
                }
            }

            atual = atual->prox;
        } while (atual != head);

		//Comportamento Parede
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

// Draw game (one frame)
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

		//Pontuação
		DrawText(TextFormat("%04i", score), 20, 20, 40, RED);

		//Menssagem Vitoria - MUDAR POR TEMPO
		if (victory) DrawText("YOU WIN", screenWidth/2 - MeasureText("YOU WIN", 40)/2, screenHeight/2 - 40, 40, BLACK);
	}
	else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, BLACK);

	EndDrawing();
}

// Unload game variables
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