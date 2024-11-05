#include "raylib.h"

#if defined(PLATFORM_WEB)
	#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define NUM_MAX_ENEMIES 50
#define FIRST_WAVE 15
#define SECOND_WAVE 25
#define THIRD_WAVE 30

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { FIRST = 0, SECOND, THIRD } OndaObstaculo;

typedef struct Barco{
	Rectangle rec;
	Vector2 speed;
	Color color;
} Barco;


//Sugestão 2 structs para cada obstávulo
typedef struct Obstaculo{
	Rectangle rec;
	Vector2 speed;
	bool active;
	Color color;
} Obstaculo;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

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

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
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
	Barcotextura = LoadTexture("resources/Barco.png");
	CenarioTexture = LoadTexture("resources/Cenario.png");

	gameOver = false;
	victory = false;
	smooth = false;
	wave = FIRST;
	activeEnemies = FIRST_WAVE;
	score = 100;
	alpha = 0;

	// Initialize player
	barco.rec.x =  20;
	barco.rec.y = (screenHeight/2) + 50;
	barco.rec.width = Barcotextura.width;
	barco.rec.height = Barcotextura.height - 25;
	barco.speed.x = 4;
	barco.speed.y = 4;

	// Initialize enemies
	for (int i = 0; i < NUM_MAX_ENEMIES; i++){
		obstaculo[i].rec.width = 15;
		obstaculo[i].rec.height = 15;
		obstaculo[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
		obstaculo[i].rec.y = GetRandomValue(screenHeight/2, screenHeight - obstaculo[i].rec.height);
		obstaculo[i].speed.x = 5;
		obstaculo[i].speed.y = 5;
		obstaculo[i].active = true;
		obstaculo[i].color = GRAY;
	}
}

// Update game (one frame)
void UpdateGame(void){
	if (!gameOver){
		
		switch (wave){
			case FIRST:{
				if (!smooth){
					alpha += 0.02f;

					if (alpha >= 1.0f) smooth = true;
				}

				if (smooth) alpha -= 0.02f;

				if (enemiesKill == activeEnemies){
					enemiesKill = 0;
					for (int i = 0; i < activeEnemies; i++){
						if (!obstaculo[i].active) obstaculo[i].active = true;
					}

					activeEnemies = SECOND_WAVE;
					wave = SECOND;
					smooth = false;
					alpha = 0.0f;
				}
			} break;
			case SECOND:{
				if (!smooth)
				{
					alpha += 0.02f;

					if (alpha >= 1.0f) smooth = true;
				}

				if (smooth) alpha -= 0.02f;

				if (enemiesKill == activeEnemies)
				{
					enemiesKill = 0;

					for (int i = 0; i < activeEnemies; i++)
					{
						if (!obstaculo[i].active) obstaculo[i].active = true;
					}

					activeEnemies = THIRD_WAVE;
					wave = THIRD;
					smooth = false;
					alpha = 0.0f;
				}
			} break;
			case THIRD:{
				if (!smooth)
				{
					alpha += 0.02f;

					if (alpha >= 1.0f) smooth = true;
				}

				if (smooth) alpha -= 0.02f;

				if (enemiesKill == activeEnemies) victory = true;

			} break;
			default: break;
		}

		// Player movement
		if (IsKeyDown(KEY_D)) barco.rec.x += barco.speed.x;
		if (IsKeyDown(KEY_A)) barco.rec.x -= barco.speed.x;
		if (IsKeyDown(KEY_W)) barco.rec.y -= barco.speed.y;
		if (IsKeyDown(KEY_S)) barco.rec.y += barco.speed.y;

		// Player collision with enemy
		for (int i = 0; i < activeEnemies; i++){
			if (CheckCollisionRecs(barco.rec, obstaculo[i].rec)){
				score -= 5;
				obstaculo[i].active = false;
				if (score == 0){
					score = 0;
					gameOver = true;
				}
			}
		}

		//Comportamento Objeto
		for (int i = 0; i < activeEnemies; i++){

			if (!obstaculo[i].active){
				obstaculo[i].active = true;
				obstaculo[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
				obstaculo[i].rec.y = GetRandomValue(0, screenHeight - obstaculo[i].rec.height);
			}

			if (obstaculo[i].active){
				obstaculo[i].rec.x -= obstaculo[i].speed.x;
				if (obstaculo[i].rec.x < 0){
					obstaculo[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
					obstaculo[i].rec.y = GetRandomValue(0, screenHeight - obstaculo[i].rec.height);
				}
			}
		}

		// Wall behaviour
		if (barco.rec.x <= 0) barco.rec.x = 0;
		if (barco.rec.x + barco.rec.width >= screenWidth) barco.rec.x = screenWidth - barco.rec.width;
		if (barco.rec.y <= 0) barco.rec.y = 0;
		if (barco.rec.y + barco.rec.height >= screenHeight) barco.rec.y = screenHeight - barco.rec.height;
	}

	else{
		if (IsKeyPressed(KEY_ENTER))
		{
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

			//Ondas Obstaculos - MUDAR POR TEMPO
			if (wave == FIRST) DrawText("FIRST WAVE", screenWidth/2 - MeasureText("FIRST WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
			else if (wave == SECOND) DrawText("SECOND WAVE", screenWidth/2 - MeasureText("SECOND WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
			else if (wave == THIRD) DrawText("THIRD WAVE", screenWidth/2 - MeasureText("THIRD WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));

			for (int i = 0; i < activeEnemies; i++){
				if (obstaculo[i].active) DrawRectangleRec(obstaculo[i].rec, obstaculo[i].color);
			}

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
	// Libere a textura
	UnloadTexture(Barcotextura);
	UnloadTexture(CenarioTexture);
}

// Update and Draw (one frame)
void UpdateDrawFrame(void){
	UpdateGame();
	DrawGame();
}
