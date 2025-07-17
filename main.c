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
Vector3 playerTargetPreviousPosition;
BoundingBox playerBoundingBox;

bool xnCollsion;
bool xpCollsion;
bool znCollsion;
bool zpCollsion;
int previousCollisonState;
float disX;
float disZ;

float gravity;
int worldBoxAmount;
int worldBoxSize;
BoundingBox worldBoxes[75];

int main(void) {

  // Initialization
  debug = true;

  screenWidth = 800;
  screenHeight = 450;
  targetFPS = 165;
  SetExitKey(KEY_NULL);
  SetTargetFPS(targetFPS);
  InitWindow(screenWidth, screenHeight, "Simple Doom Style Shooter");

  gameCamera.position = (Vector3){ 0, 5.5, 0 };
  gameCamera.target = (Vector3){ 1, 5.5, 0 };
  gameCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
  gameCamera.fovy = 100.0f;
  gameCamera.projection = CAMERA_PERSPECTIVE;

  playerSpeed = 50;
  playerWeight = 4;
  playerJumpPower = 15;
  IsPlayerJumping = false;
  playerYVel = 0;
  playerRotateSpeed = 230;
  playerSize = (Vector3){ 2, 6, 2 };
  playerPreviousPosition = gameCamera.position;
  playerTargetPreviousPosition = gameCamera.target;

  xnCollsion = false;
  xpCollsion = false;
  znCollsion = false;
  zpCollsion = false;
  previousCollisonState = 1;
  disX = 0;
  disZ = 0;

  gravity = 10;
  worldBoxAmount = 75;
  worldBoxSize = 10;
  for (int i = 0; i < worldBoxAmount; i++) {
    int x = 0;
    int z = 0;

    while (x > -worldBoxSize && x < worldBoxSize || z > -worldBoxSize && z < worldBoxSize) {
      x = floor(GetRandomValue(-100, 100 - worldBoxSize));
      z = floor(GetRandomValue(-100, 100 - worldBoxSize));
    }

    worldBoxes[i].min = (Vector3){ x, 0, z };

    worldBoxes[i].max = (Vector3){ x + worldBoxSize, worldBoxSize, z + worldBoxSize };
  }

  // Main game loop
  while (!WindowShouldClose()) {
    
    // Game logic
    deltaTime = GetFrameTime();
    fps = GetFPS();

    playerPreviousPosition = gameCamera.position;
    playerTargetPreviousPosition = gameCamera.target; // If the camera moves lets move the camera target too ;)

    UpdateCameraPro(&gameCamera,
                    (Vector3){(IsKeyDown(KEY_W) - IsKeyDown(KEY_S)) * (playerSpeed * deltaTime), 0, playerYVel * deltaTime },
                    (Vector3){(IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * (playerRotateSpeed * deltaTime), 0, 0 },
                    0.0f);

    // Force the camera to stay level
    // Without this it just does some shit idk bro
    gameCamera.target.y = gameCamera.position.y;

    // Resize bounding box around player
    playerBoundingBox.min = (Vector3){
                            gameCamera.position.x - playerSize.x / 2,
                            gameCamera.position.y - playerSize.y + 0.5,
                            gameCamera.position.z - playerSize.z / 2 };

    playerBoundingBox.max = (Vector3){
                            gameCamera.position.x + playerSize.x / 2,
                            gameCamera.position.y + 0.5,
                            gameCamera.position.z + playerSize.z / 2 };

    /* TODO: Kill the code bellow after new collisions are working

    // Have we collided with a box?
    // If so don't move
    for (int i = 0; i < worldBoxAmount; i++) {
      if (CheckCollisionBoxes(playerBoundingBox, worldBoxes[i])) {
        gameCamera.position = playerPreviousPosition;
      }
    }*/

    // Check how we are colliding with a box
    xnCollsion = false;
    xpCollsion = false;
    znCollsion = false;
    zpCollsion = false;

    for (int i = 0; i < worldBoxAmount; i++) {
      if (CheckCollisionBoxes(playerBoundingBox, worldBoxes[i])) {
        if (playerBoundingBox.min.x < worldBoxes[i].min.x) {
          xnCollsion = true;
        } 
        
        if (playerBoundingBox.max.x > worldBoxes[i].max.x) {
          xpCollsion = true;
        }
        
        if (playerBoundingBox.min.z < worldBoxes[i].min.z) {
          znCollsion = true;
        }
      
        if (playerBoundingBox.max.z > worldBoxes[i].max.z) {
          zpCollsion = true;
        }

        // Are we in a coner?
        if (xnCollsion || xpCollsion && znCollsion || zpCollsion) {
          disX = fabs(gameCamera.position.x - (worldBoxes[i].min.x + (worldBoxSize / 2)));
          disZ = fabs(gameCamera.position.z - (worldBoxes[i].min.z + (worldBoxSize / 2)));

          // Sliding on X
          if (disX > disZ) {
            znCollsion = false;
            zpCollsion = false;
            gameCamera.position.x = playerPreviousPosition.x;
            gameCamera.target.x = playerTargetPreviousPosition.x;
            if (xnCollsion) {
              // PreviousCameraPositionBeforeConerFix is used to set the camera target
              int previousCameraPositionBeforeConerFix = gameCamera.position.x;
              gameCamera.position.x = worldBoxes[i].min.x - ((playerSize.x + 0.5) / 2);
              gameCamera.target.x = gameCamera.target.x + (previousCameraPositionBeforeConerFix - gameCamera.position.x);
            } else if (xpCollsion) {
              int previousCameraPositionBeforeConerFix = gameCamera.position.x;
              gameCamera.position.x = worldBoxes[i].max.x + ((playerSize.x + 0.5) / 2);
              gameCamera.target.x = gameCamera.target.x + (previousCameraPositionBeforeConerFix - gameCamera.position.x);
            }
          } else { // Sliding on Z
            xnCollsion = false;
            xpCollsion = false;
            gameCamera.position.z = playerPreviousPosition.z;
            gameCamera.target.z = playerTargetPreviousPosition.z;
            if (znCollsion) {
              int previousCameraPositionBeforeConerFix = gameCamera.position.z;
              gameCamera.position.z = worldBoxes[i].min.z - ((playerSize.z + 0.5) / 2);
              gameCamera.target.z = gameCamera.target.z + (previousCameraPositionBeforeConerFix - gameCamera.position.z);
            } else if (zpCollsion) {
              int previousCameraPositionBeforeConerFix = gameCamera.position.z;
              gameCamera.position.z = worldBoxes[i].max.z + ((playerSize.z + 0.5) / 2);
              gameCamera.target.z = gameCamera.target.z + (previousCameraPositionBeforeConerFix - gameCamera.position.z);
            }
          }
        } else if (xnCollsion || xpCollsion) {
          gameCamera.position.x = playerPreviousPosition.x;
          gameCamera.target.x = playerTargetPreviousPosition.x;
        } else if (znCollsion || zpCollsion) {
          gameCamera.position.z = playerPreviousPosition.z;
          gameCamera.target.z = playerTargetPreviousPosition.z;
        }
      }
    }

    // Set jump flag if space has been pressed
    IsPlayerJumping = IsKeyDown(KEY_SPACE);

    // Should we jump?
    // If so jump, otherwise figure out how gravity works again
    if (IsPlayerJumping && playerBoundingBox.min.y < 0.0000001) {
      playerYVel = playerYVel + playerJumpPower;
    } else if (playerBoundingBox.min.y < 0.0000001) {
      playerYVel = 0;
      gameCamera.position.y = 5.5;
    } else {
      playerYVel = playerYVel - (gravity * deltaTime) * playerWeight;
    }

    // Rendering
    BeginDrawing();
    ClearBackground(BLACK);
      BeginMode3D(gameCamera);
      DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 200, 200 }, GRAY);
      DrawSphere((Vector3){ 5, 0, 0 }, 1, GREEN);
      DrawSphere((Vector3){ 0, 5, 0 }, 1, BLUE);
      DrawSphere((Vector3){ 0, 0, 5 }, 1, RED);
      if (debug) {
        DrawBoundingBox(playerBoundingBox, LIME);
      }
      for (int i = 0; i < worldBoxAmount; i++) {
        if (debug) {
          DrawBoundingBox(worldBoxes[i], SKYBLUE);
        }
        DrawCube((Vector3){ worldBoxes[i].min.x + worldBoxSize / 2, worldBoxSize / 2, worldBoxes[i].min.z + worldBoxSize / 2 }, worldBoxSize, worldBoxSize, worldBoxSize, LIGHTGRAY);
      }
      EndMode3D();
    DrawText("Move With WASD, Jump With Space", 10, 415, 25, RAYWHITE);
    if (debug) {
      DrawText(TextFormat("%d", fps), 10, 10, 25, RAYWHITE);
      DrawText(TextFormat("%f", playerYVel), 10, 35, 25, RAYWHITE);
      DrawText(TextFormat("%f", playerBoundingBox.min.y), 10, 60, 25, RAYWHITE);
      if (xnCollsion) {
        DrawText("XN", 10, 85, 25, RAYWHITE);
      }
      if (xpCollsion) {
        DrawText("XP", 50, 85, 25, RAYWHITE);
      }
      if (znCollsion) {
        DrawText("ZN", 90, 85, 25, RAYWHITE);
      }
      if (zpCollsion) {
        DrawText("ZP", 130, 85, 25, RAYWHITE);
      }
      DrawText(TextFormat("%f", disX), 10, 110, 25, RAYWHITE);
      DrawText(TextFormat("%f", disZ), 10, 135, 25, RAYWHITE);
    }
    EndDrawing();
  }

  // Deinitialization
  CloseWindow();

  return 0;
}