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
float CollisionDisX;
float CollisionDisZ;
float gravity;

// Keep all values as ints
// Im watching .    .
//            \______/
Vector3 wallPosition[] = {{   5,   0,   5 },
                          {  -5,   0,  -5 }};
                        
Vector3 wallSize[] =     {{   10, 12,   5 },
                          {   1,  20,   1 }};

BoundingBox wallBoundingBox[sizeof(wallPosition) / sizeof(wallPosition)[0]]; // sizeof returns the size of the array in bytes not elements

int main(void) {

  // Initialization
  debug = true;

  screenWidth = 1280;
  screenHeight = 960;
  targetFPS = 165 ;
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
  CollisionDisX = 0;
  CollisionDisZ = 0;

  gravity = 10;

  for (int i = 0; i < sizeof(wallPosition) / sizeof(wallPosition)[0]; i++) {
    wallBoundingBox[i].min = (Vector3){ wallPosition[i].x, wallPosition[i].y, wallPosition[i].z };
    wallBoundingBox[i].max = (Vector3){ wallSize[i].x + wallPosition[i].x, wallSize[i].y + wallPosition[i].y, wallSize[i].z + wallPosition[i].z };
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

    for (int i = 0; i < sizeof(wallPosition) / sizeof(wallPosition)[0]; i++) {
      if (CheckCollisionBoxes(playerBoundingBox, wallBoundingBox[i])) {
        if (playerBoundingBox.min.x < wallBoundingBox[i].min.x) {
          xnCollision = true;
        } 
        
        if (playerBoundingBox.max.x > wallBoundingBox[i].max.x) {
          xpCollision = true;
        }
        
        if (playerBoundingBox.min.z < wallBoundingBox[i].min.z) {
          znCollision = true;
        }
      
        if (playerBoundingBox.max.z > wallBoundingBox[i].max.z) {
          zpCollision = true;
        }

        // Are we in a coner?
        if (xnCollision || xpCollision && znCollision || zpCollision) {
          
          // Calculate weight
          // This weight will be used later on to scale the distance to the center of the wall from the player
          float xCollisionWeight = playerSize.x / wallSize[i].x;
          float zCollisionWeight = playerSize.z / wallSize[i].z;

          CollisionDisX = fabs(gameCamera.position.x - (wallBoundingBox[i].min.x + (wallSize[i].x / 2)));
          CollisionDisZ = fabs(gameCamera.position.z - (wallBoundingBox[i].min.z + (wallSize[i].z / 2)));

          // Apply weight
          CollisionDisX = CollisionDisX * xCollisionWeight;
          CollisionDisZ = CollisionDisZ * zCollisionWeight;

          // This offset will sometimes make movement feel jittery but its better to have this happen then to have the player get stuck in the wall
          float offset = 0.1;

          // Sliding on X
          if (CollisionDisX > CollisionDisZ) {
            znCollision = false;
            zpCollision = false;
            gameCamera.position.x = playerPreviousPosition.x;
            gameCamera.target.x = cameraTargetPreviousPosition.x;
            
            if (xnCollision) {
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
    // We do this again after colisions because yes
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
    if (IsPlayerJumping && playerBoundingBox.min.y == 0.0f) {
      playerYVel = playerYVel + playerJumpPower;
    } else if (playerBoundingBox.min.y < 0.0f) {
      playerYVel = 0;
      gameCamera.position.y = 5.5;
    } else {
      playerYVel = playerYVel - (gravity * deltaTime) * playerWeight;
    }

    // We can use this to know if the camera target has floated away
    // And fix it accordingly by extending how far the target is
    // This problem is mainly due to corner collision logic
    cameraTargetDistanceFromCamera = sqrtf(((gameCamera.position.x - gameCamera.target.x) * (gameCamera.position.x - gameCamera.target.x)) + ((gameCamera.position.z - gameCamera.target.z) * (gameCamera.position.z - gameCamera.target.z)));
    cameraAngle = atan2f(gameCamera.target.z - gameCamera.position.z, gameCamera.target.x - gameCamera.position.x);
    
    // We use 7 and 13 here because 9 and 11 would get me canceled
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
      for (int i = 0; i < sizeof(wallPosition) / sizeof(wallPosition)[0]; i++) {
        if (debug) {
          DrawBoundingBox(wallBoundingBox[i], LIME);
        }
        DrawCube((Vector3){ wallPosition[i].x + wallSize[i].x / 2, wallPosition[i].y + wallSize[i].y / 2, wallPosition[i].z + wallSize[i].z / 2 }, wallSize[i].x, wallSize[i].y, wallSize[i].z, LIGHTGRAY);
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
      DrawText(TextFormat("%0.2f", CollisionDisX), 10, 110, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", CollisionDisZ), 10, 135, 25, RAYWHITE);
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