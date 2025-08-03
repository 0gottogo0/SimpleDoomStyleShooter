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
bool IsPlayerAirborn;
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

Vector3 wallPosition[] = {{    5,    0,    5 },
                          {   -5,    0,   -5 }};
  
Vector3 wallSize[] =     {{   10,   12,    5 },
                          {    1,   20,    1 }};

BoundingBox wallBoundingBox[sizeof(wallPosition) / sizeof(wallPosition)[0]]; // sizeof returns the size of the array in bytes not elements

Vector3 floorPosition[] = {{ -100,   -1, -100 },
                           {   10,    0,   24 },
                           {   10,    1,   23 },
                           {   10,    2,   22 },
                           {   10,    3,   21 },
                           {   10,    4,   20 },
                           {  -20,    6,  -20 }};

Vector3 floorSize[] =     {{  200,    1,  200 },
                           {    5,    1,    1 },
                           {    5,    1,    1 },
                           {    5,    1,    1 },
                           {    5,    1,    1 },
                           {    5,    1,    1 },
                           {   10,    1,   10 }};

BoundingBox floorBoundingBox[sizeof(floorPosition) / sizeof(floorPosition)[0]];

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
  IsPlayerAirborn = false;
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

  for (int i = 0; i < sizeof(floorPosition) / sizeof(floorPosition)[0]; i++) {
    floorBoundingBox[i].min = (Vector3){ floorPosition[i].x, floorPosition[i].y, floorPosition[i].z };
    floorBoundingBox[i].max = (Vector3){ floorSize[i].x + floorPosition[i].x, floorSize[i].y + floorPosition[i].y, floorSize[i].z + floorPosition[i].z };
  }

  // Main game loop
  while (!WindowShouldClose()) {
    
    // Game logic
    deltaTime = GetFrameTime();
    fps = GetFPS();

    playerPreviousPosition = gameCamera.position;
    cameraTargetPreviousPosition = gameCamera.target; // If the camera moves move the camera target too

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

    // Set jump flag if space has been pressed
    IsPlayerJumping = IsKeyDown(KEY_SPACE);

    // Floor/celing collision detection
    for (int i = 0; i < sizeof(floorPosition) / sizeof(floorPosition)[0]; i++) {
      if (CheckCollisionBoxes(playerBoundingBox, floorBoundingBox[i])) {
        IsPlayerAirborn = false;

        // Are we above the floor?
        if (playerBoundingBox.max.y > floorBoundingBox[i].max.y) {
          if (IsPlayerJumping) {
            playerYVel = playerJumpPower;
          } else {
            playerYVel = 0;
            gameCamera.position.y = floorBoundingBox[i].max.y + (playerSize.y - 0.5);
            gameCamera.target.y = floorBoundingBox[i].max.y + (playerSize.y - 0.5);
          }
        } else { // We are below the floor, we pretend the floor is a ceiling (no jumping)
          playerYVel = 0;
          gameCamera.position.y = playerPreviousPosition.y;
          gameCamera.target.y = cameraTargetPreviousPosition.y;
        }
      } else {
        IsPlayerAirborn = true;
      }
    }

    // Gravity
    if (IsPlayerAirborn) {
      playerYVel = playerYVel - (gravity * deltaTime) * playerWeight;
    }

    // Resize bounding box around player
    // We do this again after colisions to see with debug wireframe things
    playerBoundingBox.min = (Vector3){
                            gameCamera.position.x - playerSize.x / 2,
                            gameCamera.position.y - playerSize.y + 0.5,
                            gameCamera.position.z - playerSize.z / 2 };

    playerBoundingBox.max = (Vector3){
                            gameCamera.position.x + playerSize.x / 2,
                            gameCamera.position.y + 0.5,
                            gameCamera.position.z + playerSize.z / 2 };

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
      if (debug) {DrawBoundingBox(playerBoundingBox, LIME);}
      for (int i = 0; i < sizeof(wallPosition) / sizeof(wallPosition)[0]; i++) {
        DrawCube((Vector3){ wallPosition[i].x + wallSize[i].x / 2, wallPosition[i].y + wallSize[i].y / 2, wallPosition[i].z + wallSize[i].z / 2 }, wallSize[i].x, wallSize[i].y, wallSize[i].z, LIGHTGRAY);
        if (debug) {DrawBoundingBox(wallBoundingBox[i], LIME);}
      }
      DrawCube((Vector3){ floorPosition[0].x + floorSize[0].x / 2, floorPosition[0].y + floorSize[0].y / 2, floorPosition[0].z + floorSize[0].z / 2 }, floorSize[0].x, floorSize[0].y, floorSize[0].z, GRAY);
      if (debug) {DrawBoundingBox(floorBoundingBox[0], LIME);}
      for (int i = 1; i < sizeof(floorPosition) / sizeof(floorPosition)[0]; i++) {
        DrawCube((Vector3){ floorPosition[i].x + floorSize[i].x / 2, floorPosition[i].y + floorSize[i].y / 2, floorPosition[i].z + floorSize[i].z / 2 }, floorSize[i].x, floorSize[i].y, floorSize[i].z, BEIGE);
        if (debug) {DrawBoundingBox(floorBoundingBox[i], LIME);}
      }
      EndMode3D();
    if (debug) {
      DrawText(TextFormat("%d", fps), 10, 10, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", playerYVel), 10, 35, 25, RAYWHITE);
      DrawText(TextFormat("%0.2f", playerBoundingBox.min.y), 10, 60, 25, RAYWHITE);
      if (xnCollision) {DrawText("XN", 10, 85, 25, RAYWHITE);}
      if (xpCollision) {DrawText("XP", 50, 85, 25, RAYWHITE);}
      if (znCollision) {DrawText("ZN", 90, 85, 25, RAYWHITE);}
      if (zpCollision) {DrawText("ZP", 130, 85, 25, RAYWHITE);}
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