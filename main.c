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
Vector3 cameraTargetPreviousPosition;
float cameraTargetDistanceFromCamera;
float cameraAngle;
BoundingBox playerBoundingBox;

bool xnCollision;
bool xpCollision;
bool znCollision;
bool zpCollision;
float CollsionDisX;
float CollsionDisZ;

float gravity;
int worldBoxAmount;
int worldBoxSize;
BoundingBox worldBoxes[50];

int main(void) {

  // Initialization
  debug = true;

  screenWidth = 1280;
  screenHeight = 960;
  targetFPS = 165;
  SetExitKey(KEY_NULL);
  SetTargetFPS(targetFPS);
  InitWindow(screenWidth, screenHeight, "Simple Doom Style Shooter");

  gameCamera.position = (Vector3){ 0, 5.5, 0 };
  gameCamera.target = (Vector3){ 10, 5.5, 0 };
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
  cameraTargetPreviousPosition = gameCamera.target;

  xnCollision = false;
  xpCollision = false;
  znCollision = false;
  zpCollision = false;
  CollsionDisX = 0;
  CollsionDisZ = 0;

  gravity = 10;
  worldBoxAmount = 50;
  worldBoxSize = 10;
  for (int i = 0; i < worldBoxAmount; i++) {
    int x = 0;
    int z = 0;

    while (x > -worldBoxSize - 10 && x < worldBoxSize && z > -worldBoxSize - 10 && z < worldBoxSize) {
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
    cameraTargetPreviousPosition = gameCamera.target; // If the camera moves lets move the camera target too ;)

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

    // Check how we are colliding with a box
    xnCollision = false;
    xpCollision = false;
    znCollision = false;
    zpCollision = false;

    for (int i = 0; i < worldBoxAmount; i++) {
      if (CheckCollisionBoxes(playerBoundingBox, worldBoxes[i])) {
        if (playerBoundingBox.min.x < worldBoxes[i].min.x) {
          xnCollision = true;
        } 
        
        if (playerBoundingBox.max.x > worldBoxes[i].max.x) {
          xpCollision = true;
        }
        
        if (playerBoundingBox.min.z < worldBoxes[i].min.z) {
          znCollision = true;
        }
      
        if (playerBoundingBox.max.z > worldBoxes[i].max.z) {
          zpCollision = true;
        }

        // Are we in a coner?
        if (xnCollision || xpCollision && znCollision || zpCollision) {
          CollsionDisX = fabs(gameCamera.position.x - (worldBoxes[i].min.x + (worldBoxSize / 2)));
          CollsionDisZ = fabs(gameCamera.position.z - (worldBoxes[i].min.z + (worldBoxSize / 2)));
          float offset = 0.05;

          // Sliding on X
          if (CollsionDisX > CollsionDisZ) {
            znCollision = false;
            zpCollision = false;
            gameCamera.position.x = playerPreviousPosition.x;
            gameCamera.target.x = cameraTargetPreviousPosition.x;
            
            if (xnCollision) {
              // Offset the player just slightly
              gameCamera.position.x -= offset;
              gameCamera.target.x -= offset;
            } else if (xpCollision) {
              gameCamera.position.x += offset;
              gameCamera.target.x += offset;
            }
          } else { // Sliding on Z
            xnCollision = false;
            xpCollision = false;
            gameCamera.position.z = playerPreviousPosition.z;
            gameCamera.target.z = cameraTargetPreviousPosition.z;
            
            if (znCollision) {
              gameCamera.position.z -= offset;
              gameCamera.target.z -= offset;
            } else if (zpCollision) {
              gameCamera.position.z += offset;
              gameCamera.target.z += offset;
            }
          }
        } else if (xnCollision || xpCollision) {
          gameCamera.position.x = playerPreviousPosition.x;
          gameCamera.target.x = cameraTargetPreviousPosition.x;
        } else if (znCollision || zpCollision) {
          gameCamera.position.z = playerPreviousPosition.z;
          gameCamera.target.z = cameraTargetPreviousPosition.z;
        }
      }
    }

    // Resize bounding box around player
    // We do this again after colisions
    playerBoundingBox.min = (Vector3){
                            gameCamera.position.x - playerSize.x / 2,
                            gameCamera.position.y - playerSize.y + 0.5,
                            gameCamera.position.z - playerSize.z / 2 };

    playerBoundingBox.max = (Vector3){
                            gameCamera.position.x + playerSize.x / 2,
                            gameCamera.position.y + 0.5,
                            gameCamera.position.z + playerSize.z / 2 };


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

    // We can use this to know if the camera target has floated away
    // and fix it accordingly by extending how far the target is
    cameraTargetDistanceFromCamera = sqrtf(((gameCamera.position.x - gameCamera.target.x) * (gameCamera.position.x - gameCamera.target.x)) + ((gameCamera.position.z - gameCamera.target.z) * (gameCamera.position.z - gameCamera.target.z)));
    cameraAngle = atan2f(gameCamera.target.z - gameCamera.position.z, gameCamera.target.x - gameCamera.position.x);
    
    // We use 7 and 13 here because 9 and 11 woudl get me canceled.
    // Also 7 and 13 are slightly smother
    if (cameraTargetDistanceFromCamera < 7 || cameraTargetDistanceFromCamera > 13) {
      gameCamera.target.x = gameCamera.position.x + cos(cameraAngle) * 10;
      gameCamera.target.z = gameCamera.position.z + sin(cameraAngle) * 10;
    }
    
    // Rendering
    BeginDrawing();
    ClearBackground(SKYBLUE);
      BeginMode3D(gameCamera);
      DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 200, 200 }, GRAY);
      if (debug) {
        DrawBoundingBox(playerBoundingBox, LIME);
      }
      for (int i = 0; i < worldBoxAmount; i++) {
        if (debug) {
          DrawBoundingBox(worldBoxes[i], LIME);
        }
        DrawCube((Vector3){ worldBoxes[i].min.x + worldBoxSize / 2, worldBoxSize / 2, worldBoxes[i].min.z + worldBoxSize / 2 }, worldBoxSize, worldBoxSize, worldBoxSize, LIGHTGRAY);
      }
      EndMode3D();
    if (debug) {
      DrawText(TextFormat("%d", fps), 10, 10, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", playerYVel), 10, 35, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", playerBoundingBox.min.y), 10, 60, 25, RAYWHITE);
      if (xnCollision) {
        DrawText("XN", 10, 85, 25, RAYWHITE);
      }
      if (xpCollision) {
        DrawText("XP", 50, 85, 25, RAYWHITE);
      }
      if (znCollision) {
        DrawText("ZN", 90, 85, 25, RAYWHITE);
      }
      if (zpCollision) {
        DrawText("ZP", 130, 85, 25, RAYWHITE);
      }
      DrawText(TextFormat("%0.2f", CollsionDisX), 10, 110, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", CollsionDisZ), 10, 135, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", gameCamera.position.x), 10, 160, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", gameCamera.position.z), 100, 160, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", gameCamera.target.x), 10, 185, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", gameCamera.target.z), 100, 185, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", cameraTargetDistanceFromCamera), 10, 210, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", cameraAngle * (180/PI)), 10, 235, 25, RAYWHITE);
    }
    DrawText("Move With WASD, Jump With Space", 10, 925, 25, RAYWHITE);
    EndDrawing();
  }

  // Deinitialization
  CloseWindow();

  return 0;
}