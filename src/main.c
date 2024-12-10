
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NUM_MAX_OBJETOS 13 
#define PRIMEIRA_ONDA 7 
#define SEGUNDA_ONDA 10 
#define TERCEIRA_ONDA 13 
#define NUM_MAX_BANHISTA 5 
#define MAX_RANKING 100 

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
} Obstaculo;

typedef struct ListObj{ 
	Obstaculo obstaculo;
	struct ListObj *prox;
} ListObj;

typedef struct Banhista{ 
	Rectangle rec;
	Vector2 speed;
	bool active;
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
	char nome[20];
	int score;
	int banhistas;
} Player;


ListObj *head = NULL;
ListObj *tail = NULL;

ListBanhista *headBanhista = NULL;
ListBanhista *tailBanhista = NULL;

BanhistaColetados *headBanhistaColetados = NULL;
BanhistaColetados *tailBanhistaColetados = NULL;


static const int screenWidth = 1000;
static const int screenHeight = 749;


static char nick[20] = {0}; 
static int nickIndex = 0; 
static bool enteringNick = false; 
static GameState currentGameState = MENU;  

int score = 0; 
int vida = 10; 
int banhistaSalvos = 0; 
static bool gameOver = false; 
static float timerBanhista = 0; 
static bool victory = false; 

static Barco barco = { 0 }; 

static OndaObstaculo wave = { 0 }; 


static float alpha = 0.0f; 
static bool smooth = false;

static int activeObstaculos = 0; 


Texture2D moeda;
Texture2D MenuInicial;
Texture2D PedraTextura;
Texture2D Barcotextura;
Texture2D FundoVitoria;
Texture2D coracaoTextura;
Texture2D CenarioTexture;
Texture2D BanhistaTextura;


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


int main(void) {
	
	InitWindow(screenWidth, screenHeight, "Petrolândia Adventure");

	
	InitGame();
	
	
	SetTargetFPS(60);

	
	while (!WindowShouldClose()) {
		
		UpdateDrawFrame();
	}
	
	
	UnloadGame();

	
	CloseWindow();
	return 0;
}


void InitGame(void) {
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

	
	for (int i = 0; i < activeObstaculos; i++) {
		inserirobstaculos(&head, &tail);
	}

	
	for (int i = 0; i < NUM_MAX_BANHISTA; i++) {
		inserirBanhista(&headBanhista, &tailBanhista);
	}
}


void limpaObstaculos(ListObj **head, ListObj **tail) {
	

    if (*head == NULL){
		return;
	} else {
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


void limparBanhistas(ListBanhista **head, ListBanhista **tail){
	
	if (*head == NULL) {
		return;
	} else {
		ListBanhista *aux = *head;
		ListBanhista *prox = *head;
		while (aux != NULL) { 
			prox = prox->prox;
			free(aux);
			aux = prox;
		}
		
		*head = NULL;
		*tail = NULL;
	}
}


void limparBanhistasColetados(BanhistaColetados **head, BanhistaColetados **tail) {
	
	if (*head == NULL) {
		return;
	} else {
		BanhistaColetados *aux = *head;
		BanhistaColetados *prox = *head;
		while (aux != NULL) { 
			prox = prox->prox;
			free(aux);
			aux = prox;
		}
		
		*head = NULL;
		*tail = NULL;
	}
}


void inserirobstaculos (ListObj **head, ListObj **tail) {
	
  	ListObj *novo = (ListObj *) malloc(sizeof(ListObj)); 

	if(novo != NULL) { 
		
		novo->prox = *head;
		novo->obstaculo.speed.x = 5;
		novo->obstaculo.speed.y = 5;
		novo->obstaculo.active = true;
		novo->obstaculo.rec.width = PedraTextura.width - 45;
		novo->obstaculo.rec.height = PedraTextura.height - 45;
		novo->obstaculo.rec.y = GetRandomValue(470, 700); 
		novo->obstaculo.rec.x = GetRandomValue(screenWidth, screenWidth + 1000); 
		
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


void inserirBanhista (ListBanhista **head, ListBanhista **tail) {
	
    ListBanhista *nova = (ListBanhista *)malloc(sizeof(ListBanhista)); 
    
	if (nova != NULL){
		
        nova->prox = NULL;
        nova->banhista.speed.x = 5;
        nova->banhista.speed.y = 5;
        nova->banhista.active = true;
        nova->banhista.rec.width = 15;
        nova->banhista.rec.height = 15;
        nova->banhista.rec.y = GetRandomValue(470, 700); 
        nova->banhista.rec.x = GetRandomValue(screenWidth, screenWidth + 1000); 

        if (*head == NULL) {
            *head = nova;
            *tail = nova;
        } else {
            (*tail)->prox = nova;
            *tail = nova;
        }
    }
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


void UpdateMenu(void) {
    if (IsKeyPressed(KEY_ENTER)) { 
        if (!enteringNick) {
            enteringNick = true; 
        } else if (nickIndex > 0) { 
            enteringNick = false;
            currentGameState = PLAY; 
        }
    }

    if (enteringNick) { 
        int key = GetKeyPressed(); 

        
        if (key >= 32 && key <= 125 && nickIndex < 19) { 
            nick[nickIndex++] = (char) key; 
            nick[nickIndex] = '\0'; 
        }

        
        if (IsKeyPressed(KEY_BACKSPACE) && nickIndex > 0) {
            nick[--nickIndex] = '\0';
        }
    } else if (IsKeyPressed(KEY_ESCAPE)) { 
        CloseWindow(); 
    } else if (IsKeyPressed(KEY_R)) { 
        currentGameState = GAME_OVER; 
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

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, LIGHTGRAY);  
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, DARKGRAY); 
        DrawText("Digite seu Nick:", boxX + 10, boxY + 10, 20, DARKBLUE);  
        DrawText(nick, boxX + 10, boxY + 40, 30, DARKBLUE);  
        DrawText("Pressione ENTER para confirmar", boxX + 10, boxY + 70, 10, GRAY); 
    }

    EndDrawing(); 
}


void UpdateGame(void) {
	
	ListObj *atual = head;
	ListBanhista *banhistaTela = headBanhista;

	if (!gameOver) { 
		
		if (IsKeyDown(KEY_D)) barco.rec.x += barco.speed.x;
		if (IsKeyDown(KEY_A)) barco.rec.x -= barco.speed.x;
		if (IsKeyDown(KEY_W)) barco.rec.y -= barco.speed.y;
		if (IsKeyDown(KEY_S)) barco.rec.y += barco.speed.y;

		switch (wave) { 
            case PRIMEIRA: { 
                if (!smooth) { 
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true; 
                }

                if (smooth) alpha -= 0.02f; 

                if (score >= 3000) {  
                    activeObstaculos = SEGUNDA_ONDA;
                    wave = SEGUNDA;
                    smooth = false;
                    alpha = 0.0f;

                    for (int i = 0; i < activeObstaculos; i++) { 
                        inserirobstaculos(&head, &tail); 
                    }
                }
            } break;
            case SEGUNDA: {
                if (!smooth) { 
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true; 
                }

                if (smooth) alpha -= 0.02f; 

                if (score >= 6000) { 
                    activeObstaculos = TERCEIRA_ONDA;
                    wave = TERCEIRA;
                    smooth = false;
                    alpha = 0.0f;					

                    for (int i = 0; i < activeObstaculos; i++){ 
                        inserirobstaculos(&head, &tail); 
                    }
                }
            } break;
            case TERCEIRA: {
                if (!smooth) { 
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true; 
                }

                if (smooth) alpha -= 0.02f; 

                if (score >= 10000) { 
					victory = true;
					
					Player player;
					strcpy(player.nome, nick);
					player.score = score;
					player.banhistas = banhistaSalvos;
					salvarRanking(player.nome, player.score, player.banhistas); 
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
							printf("%d\n", score);
						}
						removerBanhistaColetados (&headBanhistaColetados, &tailBanhistaColetados);
						atual->obstaculo.active = false;
						if (vida == 0) {
							timerBanhista = 0;
							
							Player player;
							strcpy(player.nome, nick);
							player.score = score;
							player.banhistas = banhistaSalvos;
							salvarRanking(player.nome, player.score, player.banhistas);
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
							
							Player player;
							strcpy(player.nome, nick);
							player.score = score;
							player.banhistas = banhistaSalvos;
							salvarRanking(player.nome, player.score, player.banhistas);
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
							
							Player player;
							strcpy(player.nome, nick);
							player.score = score;
							player.banhistas = banhistaSalvos;
							salvarRanking(player.nome, player.score, player.banhistas);
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

		while (banhistaTela != NULL) { 
			if (banhistaTela->banhista.active && CheckCollisionRecs (barco.rec, banhistaTela->banhista.rec)) { 
				
				inserirBanhistaColetados(&headBanhistaColetados, &tailBanhistaColetados); 
            	banhistaSalvos = countBanhistasColetados(headBanhistaColetados); 

				if (banhistaSalvos == 50) { 
					barco.speed.x -= 1;
				} else if (banhistaSalvos < 50) { 
					barco.speed.x = 4;
				}

				if (banhistaSalvos == 100) { 
					barco.speed.x -= 1;
				} else if (banhistaSalvos < 100 && banhistaSalvos > 50) { 
					barco.speed.x = 3;
				}

            	banhistaTela->banhista.active = false; 
			}

			if (!banhistaTela->banhista.active) { 
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

		if (barco.rec.x <= 0) { 
			barco.rec.x = 0;
		}

		if (barco.rec.x + barco.rec.width >= screenWidth) { 
			barco.rec.x = screenWidth - barco.rec.width; 
		}

		if (barco.rec.y <= 450) { 
			barco.rec.y = 450;
		}

		if (barco.rec.y + barco.rec.height >= 730) { 
			barco.rec.y = 730 - barco.rec.height;
		}
	} else { 
		if (IsKeyPressed(KEY_ENTER)) { 
			InitGame(); 
			gameOver = false; 
		} else if (IsKeyPressed(KEY_M)){ 
			currentGameState=MENU; 
			InitGame(); 
		} else if (IsKeyPressed(KEY_R)) { 
			currentGameState=GAME_OVER; 
			InitGame(); 
		}
	}
}


void DrawGame(void) {
    BeginDrawing(); 

    if (victory) { 
		
        DrawTexture(FundoVitoria, 0, -20, WHITE);
        DrawText("VOCÊ CHEGOU A PETROLANDIA", screenWidth / 2 - MeasureText("VOCÊ CHEGOU A PETROLANDIA", 40) / 2, screenHeight / 2 - 20, 40, BLACK);
		DrawText("M PARA VOLTAR AO MENU", 70, 600, 20, RED);
		DrawText("ESC PARA SAIR", 70, 650, 20, RED);
    } else if (!gameOver) {
        
        DrawTexture(CenarioTexture, 0, 0, WHITE);
        
        
        DrawTexture(Barcotextura, (int)barco.rec.x, (int)barco.rec.y, WHITE);

        
        if (wave == PRIMEIRA) {
            DrawText("PRIMEIRA ONDA", screenWidth/2 - MeasureText("PRIMEIRA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
		} else if (wave == SEGUNDA) {
            DrawText("SEGUNDA ONDA", screenWidth/2 - MeasureText("SEGUNDA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
		} else if (wave == TERCEIRA) {
            DrawText("TERCEIRA ONDA", screenWidth/2 - MeasureText("TERCEIRA ONDA", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
		}

        
        ListObj *atual = head;

        do { 
            if (atual->obstaculo.active) { 
                DrawTexture(PedraTextura, (int)atual->obstaculo.rec.x, (int)atual->obstaculo.rec.y, WHITE);
            }

            atual = atual->prox; 
        } while (atual != head);

        
        ListBanhista *banhistaTela = headBanhista;

        while (banhistaTela != NULL) { 
            if (banhistaTela->banhista.active) { 
                DrawTexture(BanhistaTextura, (int)banhistaTela->banhista.rec.x, (int)banhistaTela->banhista.rec.y, WHITE);
            }

            banhistaTela = banhistaTela->prox; 
        }

        
        int nickPosX = screenWidth / 2 - MeasureText(nick, 30) / 2;
        DrawText(nick, nickPosX, 20, 30, BLUE);

		DrawTexture(moeda, -5, 15, WHITE); 
        DrawText(TextFormat("%04i", score), 60, 20, 40, RED); 
		
		DrawTexture(coracaoTextura, 5, 70, WHITE); 
		DrawText(TextFormat("%03i", vida), 60, 75, 40, RED); 

        
        DrawTexture(BanhistaTextura, 820, 8, WHITE); 
        DrawText(TextFormat("%04i", banhistaSalvos), 900, 20, 40, RED); 
    } else {
        
        DrawGameOverScreen(); 
    }

    EndDrawing(); 
}


void salvarRanking(const char* nome, int score, int numBanhistas) {
    Player ranking[MAX_RANKING]; 
    int count = 0; 

    
    FILE* file = fopen("ranking.txt", "a");

    if (file != NULL) { 
        fprintf(file, "%s;%d;%d\n", nome, score, numBanhistas); 
        fclose(file); 
    }

	
    file = fopen("ranking.txt", "r");

    if (file != NULL) { 
        while (fscanf(file, "%[^;];%d;%d\n", ranking[count].nome, &ranking[count].score, &ranking[count].banhistas) != EOF && count < MAX_RANKING) {
            count++; 
        }

        fclose(file); 
    }

    
    ordenarRanking(ranking, count);

	
    file = fopen("rankingOrdenado.txt", "w");

    for (int i = 0; i < count && i < 5; i++) { 
        fprintf(file, "%s;%d;%d\n", ranking[i].nome, ranking[i].score, ranking[i].banhistas);
    }

    fclose(file); 
}


void ordenarRanking(Player ranking[], int count) {
	
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (ranking[j].score < ranking[j + 1].score || 
                (ranking[j].score == ranking[j + 1].score && ranking[j].banhistas < ranking[j + 1].banhistas)) {
                
                Player temp = ranking[j];
                ranking[j] = ranking[j + 1];
                ranking[j + 1] = temp;
            }
        }
    }
}


void removerBanhistaColetados (BanhistaColetados **head, BanhistaColetados **tail) {
	
	if (*head != NULL) {
		BanhistaColetados *aux = *head;
		*head = (*head)->prox;
		
		if (*head == NULL) {
			*tail = NULL;
		}

		free(aux); 
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
	
    BanhistaColetados *atual = head;
    int count = 0;
	
    while (atual != NULL) { 
        count++; 
        atual = atual->prox;
    }

    return count;
}


void DrawGameOverScreen(void) {
	
    FILE *file = fopen("rankingOrdenado.txt", "r");

    Player ranking[MAX_RANKING]; 
    
    
	if (file != NULL) { 
		char line[100]; 

		for (int i = 0; i < MAX_RANKING && fgets(line, sizeof(line), file); i++) { 

			char *token = strtok(line, ";"); 
			if (token != NULL) { 
				strcpy(ranking[i].nome, token);  
			}
			
			token = strtok(NULL, ";"); 
			if (token != NULL) { 
				ranking[i].score = atoi(token); 
			}
			
			token = strtok(NULL, ";"); 
			if (token != NULL) { 
				ranking[i].banhistas = atoi(token); 
			}
		} 
		fclose(file); 
	} else { 
		printf("Erro ao abrir o arquivo de ranking.\n"); 
	}

    BeginDrawing(); 

    ClearBackground(BLUE);

	DrawRectangle(50, 50, screenWidth -100, screenHeight -100, LIGHTGRAY);
	DrawRectangleLines(50, 50, screenWidth -100, screenHeight -100, DARKGRAY);

    
    for (int i = 0; i < 5; i++) {
		DrawText(ranking[i].nome, 70, 80 + 30 * i, 20, DARKBLUE);
        DrawText(TextFormat("Score: %d", ranking[i].score), 200, 80 + 30 * i, 20, DARKGREEN); 
		DrawText(TextFormat("Banhistas: %d", ranking[i].banhistas), 350, 80 + 30 * i, 20, DARKPURPLE);

		DrawText("Ranking", screenWidth / 2 - MeasureText("Ranking", 40) / 2 + 2, screenHeight / 2 - 100 + 2, 40, GRAY); 
		DrawText("Ranking", screenWidth / 2 - MeasureText("Ranking", 40) / 2, screenHeight / 2 - 100, 40, RED);     
	}	
	DrawText("M PARA VOLTAR AO MENU", 70, 600, 20, DARKBLUE);
	DrawText("ESC PARA SAIR", 70, 650, 20, DARKBLUE);

	EndDrawing(); 
}


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