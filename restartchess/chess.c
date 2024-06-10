#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_image.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2test.lib")

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define BOARD_SIZE 8
#define TILE_SIZE 46
#define WIN_CONDITION 5
#define ANIMATION_SPEED 1
#define NUM_CLOUDS 3
#define FADE_OUT_SPEED 0.01

typedef struct {
    int x, y;
    bool isBlack;
} Click;

typedef struct {
    int x, y, w, h;
} Button;

SDL_Texture* loadTexture(const char* file, SDL_Renderer* renderer) {
    SDL_Surface* loadedSurface = IMG_Load(file);
    if (!loadedSurface) {
        printf("Unable to load image %s! SDL_image Error: %s\n", file, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return texture;
}

void renderTexture(SDL_Texture* tex, SDL_Renderer* renderer, int x, int y) {
    SDL_Rect dst = { x, y, 0, 0 };
    SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(renderer, tex, NULL, &dst);
}

void renderBoard(SDL_Renderer* renderer, SDL_Texture* boardTexture) {
    renderTexture(boardTexture, renderer, WINDOW_WIDTH / 500 - TILE_SIZE * BOARD_SIZE / 200, WINDOW_HEIGHT / 500 - TILE_SIZE * BOARD_SIZE / 200);
}

bool checkWinCondition(const Click clicks[], int clickCount, int x, int y, bool isBlack) {
    int directions[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };

    for (int d = 0; d < 4; d++) {
        int count = 1;

        for (int step = 1; step < WIN_CONDITION; step++) {
            int nx = x + directions[d][0] * step * TILE_SIZE;
            int ny = y + directions[d][1] * step * TILE_SIZE;
            bool found = false;
            for (int i = 0; i < clickCount; i++) {
                if (clicks[i].x == nx && clicks[i].y == ny && clicks[i].isBlack == isBlack) {
                    count++;
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }

        for (int step = 1; step < WIN_CONDITION; step++) {
            int nx = x - directions[d][0] * step * TILE_SIZE;
            int ny = y - directions[d][1] * step * TILE_SIZE;
            bool found = false;
            for (int i = 0; i < clickCount; i++) {
                if (clicks[i].x == nx && clicks[i].y == ny && clicks[i].isBlack == isBlack) {
                    count++;
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }

        if (count >= WIN_CONDITION) return true;
    }
    return false;
}

void resetGame(Click clicks[], int* clickCount) {
    *clickCount = 0;
    memset(clicks, 0, sizeof(Click) * 64);
}

void renderButton(SDL_Renderer* renderer, SDL_Texture* buttonTexture, const Button button) {
    renderTexture(buttonTexture, renderer, button.x, button.y);
}

bool isButtonClicked(const Button button, int x, int y) {
    return x > button.x && x < button.x + button.w && y > button.y && y < button.y + button.h;
}

void renderMenu(SDL_Renderer* renderer, SDL_Texture* backgroundTexture, SDL_Texture* startTexture, SDL_Texture* exitTexture, const Button startButton, const Button exitButton) {
    renderTexture(backgroundTexture, renderer, 0, 0);

    renderButton(renderer, startTexture, startButton);
    renderButton(renderer, exitTexture, exitButton);
}

void renderCloud(SDL_Renderer* renderer, SDL_Texture* cloudTexture, int x, int y) {
    renderTexture(cloudTexture, renderer, x, y);
}

void renderBackground(SDL_Renderer* renderer, SDL_Texture* backgroundTexture, float alpha) {
    SDL_SetTextureAlphaMod(backgroundTexture, (Uint8)(alpha * 255));
    renderTexture(backgroundTexture, renderer, 0, 0);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Concave Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool isRunning = true;
    SDL_Event event;
    int clickCount = 0;
    Click clicks[64];
    bool inGame = false;
    bool animating = false;
    float fadeAlpha = 1.0f;

    Button startButton = { WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 25, 100, 50 };
    Button exitButton = { WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 50, 100, 50 };

    SDL_Texture* cloudTexture = loadTexture("C:\\Users\\user\\Desktop\\cloud.png", renderer);
    SDL_Texture* backgroundTexture = loadTexture("C:\\Users\\user\\Desktop\\background.jpg", renderer);

    SDL_Texture* boardTexture = loadTexture("C:\\Users\\user\\Desktop\\pan.png", renderer);
    SDL_Texture* blackStoneTexture = loadTexture("C:\\Users\\user\\Desktop\\black.png", renderer);
    SDL_Texture* whiteStoneTexture = loadTexture("C:\\Users\\user\\Desktop\\white.png", renderer);
    SDL_Texture* startTexture = loadTexture("C:\\Users\\user\\Desktop\\start.png", renderer);
    SDL_Texture* exitTexture = loadTexture("C:\\Users\\user\\Desktop\\exit.png", renderer);

    int cloudX[NUM_CLOUDS];
    int cloudY[NUM_CLOUDS];
    for (int i = 0; i < NUM_CLOUDS; i++) {
        cloudX[i] = i * (WINDOW_WIDTH / NUM_CLOUDS);
        cloudY[i] = WINDOW_HEIGHT / 2 + (rand() % (WINDOW_HEIGHT / 4) - WINDOW_HEIGHT / 6);
    }

    SDL_RenderClear(renderer);

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int clickX = event.button.x;
                int clickY = event.button.y;

                if (!inGame && !animating) {
                    if (isButtonClicked(startButton, clickX, clickY)) {
                        animating = true;
                    }
                    else if (isButtonClicked(exitButton, clickX, clickY)) {
                        isRunning = false;
                    }
                }
                else if (inGame) {
                    int gridX = (clickX / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2;
                    int gridY = (clickY / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2;

                    int potentialPoints[4][2] = {
                        {gridX, gridY},
                        {gridX - TILE_SIZE, gridY},
                        {gridX, gridY - TILE_SIZE},
                        {gridX - TILE_SIZE, gridY - TILE_SIZE}
                    };

                    double minDist = 99999999;
                    int closestX = gridX, closestY = gridY;
                    for (int i = 0; i < 4; i++) {
                        int px = potentialPoints[i][0];
                        int py = potentialPoints[i][1];
                        double dist = sqrt((px - clickX) * (px - clickX) + (py - clickY) * (py - clickY));
                        if (dist < minDist) {
                            minDist = dist;
                            closestX = px;
                            closestY = py;
                        }
                    }

                    gridX = closestX;
                    gridY = closestY;

                    bool positionOccupied = false;
                    for (int i = 0; i < clickCount; i++) {
                        if (clicks[i].x == gridX && clicks[i].y == gridY) {
                            positionOccupied = true;
                            break;
                        }
                    }

                    if (!positionOccupied) {
                        clicks[clickCount] = (Click){ .x = gridX, .y = gridY, .isBlack = (clickCount % 2 == 0) };
                        clickCount++;
                    }
                    else {
                        printf("이미 돌이 있는 위치입니다. 다른 위치를 선택해주세요.\n");
                    }
                }
            }
        }

        SDL_RenderClear(renderer);

        if (!inGame && !animating) {
            renderMenu(renderer, backgroundTexture, startTexture, exitTexture, startButton, exitButton);

            for (int i = 0; i < NUM_CLOUDS; i++) {
                cloudX[i] += ANIMATION_SPEED;
                if (cloudX[i] > WINDOW_WIDTH) {
                    cloudX[i] = -200;
                }
                renderCloud(renderer, cloudTexture, cloudX[i], cloudY[i]);
            }
        }

        if (animating) {
            fadeAlpha -= FADE_OUT_SPEED;
            if (fadeAlpha <= 0.0f) {
                fadeAlpha = 0.0f;
                animating = false;
                inGame = true;
            }
            renderBackground(renderer, backgroundTexture, fadeAlpha);
        }

        if (inGame) {
            renderBoard(renderer, boardTexture);
            for (int i = 0; i < clickCount; i++) {
                SDL_Texture* stoneTexture = clicks[i].isBlack ? blackStoneTexture : whiteStoneTexture;
                renderTexture(stoneTexture, renderer, clicks[i].x - TILE_SIZE / 2 + 15, clicks[i].y - TILE_SIZE / 2 + 19);
            }
        }

        SDL_RenderPresent(renderer);

        if (inGame) {
            for (int i = 0; i < clickCount; i++) {
                if (checkWinCondition(clicks, clickCount, clicks[i].x, clicks[i].y, clicks[i].isBlack)) {
                    const char* winner = clicks[i].isBlack ? "White" : "Black";
                    char message[100];
                    snprintf(message, sizeof(message), "%s Wins!", winner);
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", message, window);
                    resetGame(clicks, &clickCount);
                    inGame = false;
                    fadeAlpha = 1.0f;
                    break;
                }
            }
        }
    }

    SDL_DestroyTexture(cloudTexture);
    SDL_DestroyTexture(backgroundTexture);

    SDL_DestroyTexture(boardTexture);
    SDL_DestroyTexture(blackStoneTexture);
    SDL_DestroyTexture(whiteStoneTexture);
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(exitTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
