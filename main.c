#include <stdio.h>
#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"

// Constructor
bool debug;

int screenWidth;
int screenHeight;
int targetFPS;
float deltaTime;
int fps;

Camera3D gameCamera;

float playerSpeed;
float playerWeight;
float playerJumpPower;
bool IsPlayerJumping;
float playerYVel;
float playerRotateSpeed;
Vector3 playerSize;
Vector3 playerPreviousPosition;
BoundingBox playerBoundingBox;

float gravity;
int worldBoxAmount;
int worldBoxSize;
BoundingBox worldBoxes[10];

int main(void) {

  // Initialization
  debug = true;

  screenWidth = 800;
  screenHeight = 450;
  targetFPS = 60;
  SetExitKey(KEY_NULL);
  SetTargetFPS(targetFPS);
  InitWindow(screenWidth, screenHeight, "Simple Doom Style Shooter");

  gameCamera.position = (Vector3){ 0, 5.5, 0 };
  gameCamera.target = (Vector3){ 1, 5.5, 0 };
  gameCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
  gameCamera.fovy = 90.0f;
  gameCamera.projection = CAMERA_PERSPECTIVE;

  playerSpeed = 50;
  playerWeight = 0.3;
  playerJumpPower = 1;
  IsPlayerJumping = false;
  playerYVel = 0;
  playerRotateSpeed = 230;
  playerSize = (Vector3){ 2, 6, 2 };
  playerPreviousPosition = gameCamera.position;

  gravity = 9.81;
  worldBoxAmount = 10;
  worldBoxSize = 10;
  for (int i = 0; i < worldBoxAmount; i++) {
    int x = GetRandomValue(-50, 50 - worldBoxSize);
    int z = GetRandomValue(-50, 50 - worldBoxSize);

    worldBoxes[i].min = (Vector3){ x, 0, z };

    worldBoxes[i].max = (Vector3){ x + worldBoxSize, worldBoxSize, z + worldBoxSize };
  }

  // Main game loop
  while (!WindowShouldClose()) {
    
    // Game logic
    deltaTime = GetFrameTime();
    fps = GetFPS();

    playerPreviousPosition = gameCamera.position;

    UpdateCameraPro(&gameCamera,
                    (Vector3){(IsKeyDown(KEY_W) - IsKeyDown(KEY_S)) * (playerSpeed * deltaTime), 0, playerYVel },
                    (Vector3){(IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * (playerRotateSpeed * deltaTime), 0, 0 },
                    0.0f);

    playerBoundingBox.min = (Vector3){
                            gameCamera.position.x - playerSize.x / 2,
                            gameCamera.position.y - playerSize.y + 0.5,
                            gameCamera.position.z - playerSize.z / 2 };

    playerBoundingBox.max = (Vector3){
                            gameCamera.position.x + playerSize.x / 2,
                            gameCamera.position.y + 0.5,
                            gameCamera.position.z + playerSize.z / 2 };

    // Have we collided with a box?
    // If so don't move
    for (int i = 0; i < worldBoxAmount; i++) {
      if (CheckCollisionBoxes(playerBoundingBox, worldBoxes[i])) {
        gameCamera.position = playerPreviousPosition;
      }
    }

    IsPlayerJumping = IsKeyPressed(KEY_SPACE);

    if (IsPlayerJumping) {
      playerYVel = playerYVel + playerJumpPower;
    } else if (playerBoundingBox.min.y < 0.1) {
      playerYVel = 0;
      gameCamera.position.y = 5.5;
      gameCamera.target.y = 5.5;
    } else {
      playerYVel = playerYVel - (gravity * playerWeight * deltaTime);
    }

    // Rendering
    BeginDrawing();
    ClearBackground(BLACK);
      BeginMode3D(gameCamera);
      DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 100, 100 }, GRAY);
      DrawSphere((Vector3){ 5, 0, 0 }, 1, GREEN);
      DrawSphere((Vector3){ 0, 5, 0 }, 1, BLUE);
      DrawSphere((Vector3){ 0, 0, 5 }, 1, RED);
      if (debug) {
        DrawBoundingBox(playerBoundingBox, LIME);
      }
      for (int i = 0; i < worldBoxAmount; i++) {
        DrawBoundingBox(worldBoxes[i], SKYBLUE);
      }
      EndMode3D();
    DrawText("Move With WASD, Jump With Space", 10, 415, 25, RAYWHITE);
    DrawText(TextFormat("%d", fps), 10, 10, 25, RAYWHITE);
    DrawText(TextFormat("%f", playerYVel), 10, 35, 25, RAYWHITE);
    DrawText(TextFormat("%f", playerBoundingBox.min.y), 10, 60, 25, RAYWHITE);
    EndDrawing();
  }

  // Deinitialization
  CloseWindow();

  return 0;
}