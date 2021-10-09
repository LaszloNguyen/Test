// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "raylib.h"
#include "raymath.h"

#define G 300
#define PLAYER_JUMP_SPD 250.0f
#define PLAYER_HOR_SPD 1.0f
#define CHAR_HEIGHT 60.0f
#define CHAR_WIDTH 30.0f

typedef struct Player {
    Texture2D texture;
    Vector2 position;
    Rectangle playerRect;
    float dx, dy;
    bool canJump;
    int face;
    int frame;
} Player;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;

void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Test");

    // - Player
    Player player = { 0 };
    player.texture = LoadTexture("./resources/scarfy.png");
    player.position = Vector2{ 400, 400 - CHAR_HEIGHT};
    player.playerRect = { player.position.x, player.position.y ,CHAR_WIDTH, CHAR_HEIGHT };
    player.dx = 0;
    player.dy = 0;
    player.canJump = false;
    player.face = 1;

    float frameWidth = (float)(player.texture.width / 6);

    // - Background and obstacles
    EnvItem envItems[] = {
        {{ -100, 400, 1000, 200 }, 1, GRAY },
        {{ 250, 200, 200, 30 }, 1, GRAY },
        {{ 100, 300, 100, 30 }, 1, GRAY },
        {{ 550, 100, 250, 30 }, 1, GRAY }
    };
    int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float timer = GetFrameTime();
        UpdatePlayer(&player, envItems, envItemsLength, timer);

        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

            player.playerRect = { player.position.x, player.position.y, CHAR_WIDTH, CHAR_HEIGHT };

            DrawTexturePro(
                player.texture,
                Rectangle{ player.frame*frameWidth, 0, player.face*frameWidth, (float)player.texture.height },
                player.playerRect,
                Vector2{ 0, 0 },
                1, 
                RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(player.texture); // Texture unloading
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta)
{
    if (player->frame > 5) {
        player->frame = 0;
    }
    if (IsKeyDown(KEY_LEFT)) {
        player->dx -= PLAYER_HOR_SPD;
        if (player->dx < -5) {
            player->dx = -5;
        }
        player->face = -1;
        player->frame += 1;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        player->dx += PLAYER_HOR_SPD;
        if (player->dx > 5) {
            player->dx = 5;
        }
        player->face = 1;
        player->frame += 1;
    }
    if (IsKeyDown(KEY_UP) && player->canJump) {
        player->dy = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }
    if (IsKeyReleased(KEY_LEFT) || IsKeyReleased(KEY_RIGHT)) { // Reset dx if LEFT/RIGHT key released
        player->dx = 0;
    }

    bool collision = false;
    int hitObstacle = 0;
    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem* ei = envItems + i;
        float eix = ei->rect.x, eiy = ei->rect.y;
        float eiw = ei->rect.width, eih = ei->rect.height;
        float px = player->position.x, py = player->position.y;
        collision = CheckCollisionRecs(player->playerRect, ei->rect);

        if (!collision || !ei->blocking) { //Check for collision or if obstacle is blocking
            continue;
        }

        if (px + CHAR_WIDTH > eix && px < eix + eiw) {
            // Check if bumping from below
            if (py < eiy + eih &&
                py > eiy &&
                player->dy < 0.0f)
            {
                player->position.y = eiy + eih;
                py = ei->rect.y + eih;
                player->dy = 0.0f;
            }
            // Check if landing from the top
            if (py + CHAR_HEIGHT > eiy &&
                py < eiy &&
                player->dy > 0)
            {
                player->position.y = eiy - CHAR_HEIGHT;
                py = eiy - CHAR_HEIGHT;
                player->dy = 0.0f;
                hitObstacle = 1;
            }
        }

        if (py + CHAR_HEIGHT > eiy && py < eiy + eih) {
            // Check if contact left edge
            // If p
            if (px < eix + eiw &&
                px + CHAR_WIDTH > eix + eiw &&
                player->dx < 0)
            {
                player->position.x = eix + eiw;
                px = eix + eiw;
                player->dx = 0;
            }
            // Check if contact right edge - Possible bug when obstacle smaller than player
            else if (px + CHAR_WIDTH > eix &&
                px < eix &&
                player->dx > 0) 
            {
                player->position.x = eix - CHAR_WIDTH;
                px = eix - CHAR_WIDTH;
                player->dx = 0;
            }
        }
    }

    if (!hitObstacle) {
        player->dy += G * delta;
        player->canJump = false;
    }
    else player->canJump = true;

    player->position.x += player->dx;
    player->position.y += player->dy * delta;

    // - Confine player in bounds
    if (player->position.x < 0) player->position.x = 0; //LEFT BOUND CHECK
    if (player->position.x + CHAR_WIDTH > GetScreenWidth() ) player->position.x = GetScreenWidth() - CHAR_WIDTH; //RIGHT BOUND CHECK
    if (player->position.y < 0) player->position.y = 0; // UPPER BOUND CHECK
}
