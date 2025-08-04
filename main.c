#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define MAX_ASTEROIDS 20
#define PLAYER_SPEED 5
#define PLAYER_LIVES 3

typedef enum { MENU, GAMEPLAY, GAMEOVER } GameScreen;

typedef struct {
    Vector2 position;
    float speed;
    float sizeScale;
    bool active;
    Rectangle collider;
} Asteroid;

int screenWidth = 800;
int screenHeight = 600;

Texture2D playerTexture;
Texture2D asteroidTexture;
Texture2D backgroundTexture;
Music bgMusic;
Sound hitSound;
Sound vibrateSound;

Vector2 playerPos;
int score = 0;
int level = 1;
int lives = PLAYER_LIVES;
bool collision = false;
bool flashRed = false;
float flashTimer = 0;
float shakeTimer = 0;
float hitCooldown = 0.0f;

Asteroid asteroids[MAX_ASTEROIDS];

void InitAsteroids() {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        float scale = GetRandomValue(50, 100) / 100.0f;
        asteroids[i].sizeScale = scale;
        asteroids[i].position = (Vector2){ GetRandomValue(0, screenWidth - asteroidTexture.width * scale), GetRandomValue(-1000, -50) };
        asteroids[i].speed = GetRandomValue(1, 2) + (level * 0.3f);
        asteroids[i].active = true;
        asteroids[i].collider = (Rectangle){asteroids[i].position.x, asteroids[i].position.y, asteroidTexture.width * scale, asteroidTexture.height * scale};
    }
}

void UpdateAsteroids() {
    int activeAsteroidCount = 5 + level * 2;
    if (activeAsteroidCount > MAX_ASTEROIDS) activeAsteroidCount = MAX_ASTEROIDS;

    for (int i = 0; i < activeAsteroidCount; i++) {
        if (asteroids[i].active) {
            asteroids[i].position.y += asteroids[i].speed;
            asteroids[i].collider.x = asteroids[i].position.x;
            asteroids[i].collider.y = asteroids[i].position.y;

            if (asteroids[i].position.y > screenHeight) {
                float scale = GetRandomValue(50, 100) / 100.0f;
                asteroids[i].sizeScale = scale;
                asteroids[i].position.y = GetRandomValue(-200, -50);
                asteroids[i].position.x = GetRandomValue(0, screenWidth - asteroidTexture.width * scale);
                asteroids[i].speed = GetRandomValue(1, 2) + (level * 0.3f);
                score++;
                if (score % 10 == 0) level++;
            }

            Rectangle playerRect = {
                playerPos.x + playerTexture.width * 0.25f,
                playerPos.y + playerTexture.height * 0.25f,
                playerTexture.width * 0.5f,
                playerTexture.height * 0.5f
            };

            Rectangle asteroidRect = {
                asteroids[i].position.x + (asteroidTexture.width * asteroids[i].sizeScale) * 0.25f,
                asteroids[i].position.y + (asteroidTexture.height * asteroids[i].sizeScale) * 0.25f,
                (asteroidTexture.width * asteroids[i].sizeScale) * 0.5f,
                (asteroidTexture.height * asteroids[i].sizeScale) * 0.5f
            };

            if (CheckCollisionRecs(playerRect, asteroidRect) && hitCooldown <= 0.0f) {
                lives--;
                if (lives == 0) {
                    PlaySound(hitSound);
                    collision = true;
                } else {
                    PlaySound(vibrateSound);
                    flashRed = true;
                    flashTimer = 0.2f;
                    shakeTimer = 0.3f;
                    hitCooldown = 1.0f;
                }
            }
        }
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "Asteroid Dodger");
    InitAudioDevice();

    playerTexture = LoadTexture("spaceship.png");
    asteroidTexture = LoadTexture("asteroid.png");
    backgroundTexture = LoadTexture("space_background.png");
    bgMusic = LoadMusicStream("background.mp3");
    hitSound = LoadSound("hit.wav");
    vibrateSound = LoadSound("vibrate.wav");

    playerPos = (Vector2){ screenWidth / 2 - playerTexture.width / 2, screenHeight - playerTexture.height - 20 };

    SetTargetFPS(60);
    srand(time(NULL));
    InitAsteroids();
    PlayMusicStream(bgMusic);

    GameScreen screen = MENU;

    while (!WindowShouldClose()) {
        UpdateMusicStream(bgMusic);
        float shakeOffsetX = 0;
        float shakeOffsetY = 0;

        if (flashRed) {
            flashTimer -= GetFrameTime();
            if (flashTimer <= 0) flashRed = false;
        }

        if (shakeTimer > 0) {
            shakeTimer -= GetFrameTime();
            shakeOffsetX = GetRandomValue(-5, 5);
            shakeOffsetY = GetRandomValue(-5, 5);
        }

        if (hitCooldown > 0.0f) hitCooldown -= GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);

        switch (screen) {
            case MENU:
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                DrawText("ASTEROID DODGER", screenWidth/2 - 170, 100, 40, WHITE);
                DrawText("Press [ENTER] to Start", screenWidth/2 - 140, 300, 20, YELLOW);
                DrawText("Use Arrow Keys to Move (Up, Down, Left, Right)", screenWidth/2 - 260, 350, 20, GRAY);
                // Draw shadow (black, offset by 1 pixel)
DrawText("Created by Sourov", screenWidth - 230, screenHeight - 31, 20, BLACK);

// Draw main text (bright yellow)
DrawText("Created by Sourov", screenWidth - 230, screenHeight - 30, 20, GREEN);

                if (IsKeyPressed(KEY_ENTER)) {
                    score = 0;
                    level = 1;
                    lives = PLAYER_LIVES;
                    collision = false;
                    playerPos.x = screenWidth / 2 - playerTexture.width / 2;
                    InitAsteroids();
                    screen = GAMEPLAY;
                }
                break;

            case GAMEPLAY:
                DrawTexture(backgroundTexture, shakeOffsetX, shakeOffsetY, WHITE);
                DrawTexture(playerTexture, playerPos.x + shakeOffsetX, playerPos.y + shakeOffsetY, WHITE);

                if (IsKeyDown(KEY_LEFT) && playerPos.x > 0) playerPos.x -= PLAYER_SPEED;
                if (IsKeyDown(KEY_RIGHT) && playerPos.x < screenWidth - playerTexture.width) playerPos.x += PLAYER_SPEED;
                if (IsKeyDown(KEY_UP) && playerPos.y > 0) playerPos.y -= PLAYER_SPEED;
                if (IsKeyDown(KEY_DOWN) && playerPos.y < screenHeight - playerTexture.height) playerPos.y += PLAYER_SPEED;

                for (int i = 0; i < MAX_ASTEROIDS; i++) {
                    if (asteroids[i].active) {
                        DrawTextureEx(asteroidTexture, (Vector2){asteroids[i].position.x + shakeOffsetX, asteroids[i].position.y + shakeOffsetY}, 0, asteroids[i].sizeScale, WHITE);
                    }
                }

                UpdateAsteroids();

                DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
                DrawText(TextFormat("Level: %d", level), 10, 40, 20, WHITE);
                DrawText(TextFormat("Lives: %d", lives), 10, 70, 20, RED);

                if (flashRed) DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, 0.3f));

                if (collision) screen = GAMEOVER;
                break;

            case GAMEOVER:
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                DrawText("GAME OVER", screenWidth/2 - 120, 150, 40, RED);
                DrawText(TextFormat("Final Score: %d", score), screenWidth/2 - 110, 220, 30, WHITE);
                DrawText("Press [ENTER] to Return to Menu", screenWidth/2 - 180, 300, 20, YELLOW);
                if (IsKeyPressed(KEY_ENTER)) screen = MENU;
                break;
        }

        EndDrawing();
    }

    UnloadTexture(playerTexture);
    UnloadTexture(asteroidTexture);
    UnloadTexture(backgroundTexture);
    UnloadMusicStream(bgMusic);
    UnloadSound(hitSound);
    UnloadSound(vibrateSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
