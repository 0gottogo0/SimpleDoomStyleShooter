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
float playerRotateSpeed;
Vector3 playerSize;
Vector3 playerPreviousPosition;
BoundingBox playerBoundingBox;
bool xCollision;
bool zCollision;

int worldBoxAmount;
int worldBoxSize;
BoundingBox worldBoxes[20];

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

  playerSpeed = 30;
  playerRotateSpeed = 200;
  playerSize = (Vector3){ 2, 6, 2 };
  playerPreviousPosition = gameCamera.position;

  worldBoxAmount = 20;
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
                    (Vector3){(IsKeyDown(KEY_W) - IsKeyDown(KEY_S)) * (playerSpeed * deltaTime), 0, 0 },
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

    xCollision = false;
    zCollision = false;

    for (int i = 0; i < worldBoxAmount; i++) {
      if (CheckCollisionBoxes(playerBoundingBox, worldBoxes[i])) {
        gameCamera.position = playerPreviousPosition;
      }
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
    DrawText("Move With WASD", 10, 415, 25, RAYWHITE);
    DrawText(TextFormat("%i", fps), 10, 10, 25, RAYWHITE);
    EndDrawing();
  }

  // Deinitialization
  CloseWindow();

  return 0;
}