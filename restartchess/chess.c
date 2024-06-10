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
#define BOARD_SIZE 5
#define TILE_SIZE 45
#define WIN_CONDITION 5

// Click 구조체 정의: 클릭 위치와 돌 색상 저장
typedef struct {
    int x, y;
    bool isBlack;
} Click;

// 텍스처 로드 함수: 파일 경로와 렌더러를 받아 텍스처 생성
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

// 텍스처 렌더링 함수: 텍스처를 주어진 위치에 렌더링
void renderTexture(SDL_Texture* tex, SDL_Renderer* renderer, int x, int y) {
    SDL_Rect dst = { x, y, 0, 0 };
    SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(renderer, tex, NULL, &dst);
}

// 보드 렌더링 함수: 보드 텍스처를 로드하여 화면에 렌더링
void renderBoard(SDL_Renderer* renderer) {
    SDL_Texture* boardTexture = loadTexture("C:\\Users\\user\\Desktop\\pan.png", renderer);
    if (boardTexture) {
        renderTexture(boardTexture, renderer, WINDOW_WIDTH / 500 - TILE_SIZE * BOARD_SIZE / 200, WINDOW_HEIGHT / 500 - TILE_SIZE * BOARD_SIZE / 200);
        SDL_DestroyTexture(boardTexture);
    }
}

// 승리 조건 확인 함수: 클릭 기록과 현재 위치를 기반으로 승리 여부 확인
bool checkWinCondition(Click clicks[], int clickCount, int x, int y, bool isBlack) {
    // 네 방향: 가로, 세로, 대각선(좌상-우하), 대각선(우상-좌하)
    int directions[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };

    for (int d = 0; d < 4; d++) {
        int count = 1;

        // 현재 돌을 기준으로 한 방향으로 WIN_CONDITION만큼 연속된 돌이 있는지 확인
        for (int step = 1; step < WIN_CONDITION; step++) {
            int nx = x + directions[d][0] * step * TILE_SIZE;
            int ny = y + directions[d][1] * step * TILE_SIZE;
            for (int i = 0; i < clickCount; i++) {
                if (clicks[i].x == nx && clicks[i].y == ny && clicks[i].isBlack == isBlack) {
                    count++;
                    break;
                }
            }
        }

        // 반대 방향으로도 확인
        for (int step = 1; step < WIN_CONDITION; step++) {
            int nx = x - directions[d][0] * step * TILE_SIZE;
            int ny = y - directions[d][1] * step * TILE_SIZE;
            for (int i = 0; i < clickCount; i++) {
                if (clicks[i].x == nx && clicks[i].y == ny && clicks[i].isBlack == isBlack) {
                    count++;
                    break;
                }
            }
        }

        // WIN_CONDITION 이상 연속된 돌이 있으면 승리
        if (count >= WIN_CONDITION) return true;
    }
    return false;
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

    SDL_RenderClear(renderer);

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int clickX = event.button.x;
                int clickY = event.button.y;
                printf("x %d y %d\n", clickX, clickY);

                // 가까운 네 점의 좌표 계산
                int gridX = (clickX / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2;
                int gridY = (clickY / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2;
                printf("gridx %d gridy %d\n", gridX + 15, gridY + 19);

                int potentialPoints[4][2] = {
                    {gridX, gridY},
                    {gridX - TILE_SIZE, gridY},
                    {gridX, gridY - TILE_SIZE},
                    {gridX - TILE_SIZE, gridY - TILE_SIZE}
                };

                // 가장 가까운 점 찾기
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

                // 클릭된 위치에 돌이 이미 있는지 확인
                bool positionOccupied = false;
                for (int i = 0; i < clickCount; i++) {
                    if (clicks[i].x == gridX && clicks[i].y == gridY) {
                        positionOccupied = true;
                        break;
                    }
                }

                // 빈 위치면 돌을 놓음
                if (!positionOccupied) {
                    clicks[clickCount] = (Click){ .x = gridX, .y = gridY, .isBlack = (clickCount % 2 == 0) };
                    clickCount++;
                }
                else {
                    printf("이미 돌이 있는 위치입니다. 다른 위치를 선택해주세요.\n");
                }
            }
        }

        SDL_RenderClear(renderer);

        renderBoard(renderer);

        // 놓인 모든 돌을 렌더링
        for (int i = 0; i < clickCount; i++) {
            SDL_Texture* piece = loadTexture(clicks[i].isBlack ? "C:\\Users\\user\\Desktop\\black.png" : "C:\\Users\\user\\Desktop\\white.png", renderer);
            if (piece) {
                renderTexture(piece, renderer, clicks[i].x, clicks[i].y);
                SDL_DestroyTexture(piece);
            }
        }

        SDL_RenderPresent(renderer);

        // 승리 조건 확인
        if (clickCount > 0 && checkWinCondition(clicks, clickCount, clicks[clickCount - 1].x, clicks[clickCount - 1].y, clicks[clickCount - 1].isBlack)) {
            const char* winner = clicks[clickCount - 1].isBlack ? "White" : "Black";
            char message[50];
            snprintf(message, sizeof(message), "%s wins!", winner);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", message, window);
            isRunning = false;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    IMG_Quit();

    return 0;
}
