#include "main.h"

#include <stdlib.h>
#include <math.h>


#define MAX_PLAYER_BULLET_POOL 20
#define MAX_ENEMIES_POOL 20
#define SCROLLING_BACKGROUND_SPEED 80.0f

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------

static int screenWidth = 640;
static int screenHeight = 480;

static Texture2D backgroundTexture2D;
static Texture2D playerTexture2D;
static Texture2D playerBulletTexture2D;
static Texture2D bottomTexture2D;
static Texture2D enemy1Texture2D;

static GameObject player;
static GameObject* playerBullets;
static GameObject* enemies;

static ScrollableObject background1;
static ScrollableObject background2;

static Widget bottomUI;

static Music backgroundMC;


int main(void)
{
    // Initialization
    InitWindow(screenWidth, screenHeight, "AIV - 1945");
    InitAudioDevice();
    SetTargetFPS(60);

    ChangeDirectory(GetApplicationDirectory());
    TraceLog(LOG_DEBUG, "Change directory to: %s", GetApplicationDirectory());


    InitGame();
    GameLoop();
    ShoutDown();

    return 0;
}

void InitGame()
{
    backgroundTexture2D = LoadTexture("assets/map/water.png");
    playerTexture2D = LoadTexture("assets/player/myplane_strip3.png");
    playerBulletTexture2D = LoadTexture("assets/player/bullet.png");
    enemy1Texture2D = LoadTexture("assets/enemy/enemy1_strip3.png");
    bottomTexture2D = LoadTexture("assets/ui/bottom.png");

    background1.texture = &backgroundTexture2D;
    background1.posX = 0;
    background1.posY = 0;

    background2 = background1; 
    background2.posY -= GetScreenHeight();

    player.texture = &playerTexture2D;
    player.pos = (Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
    player.pivotOffset = (Vector2){(float)playerTexture2D.height  * 0.5f, (float)playerTexture2D.height * 0.5f};
    player.velocityNomalize = (Vector2){0, 0};
    player.width = 65;
    player.height = 65;
    player.anim = CreateAnimation2D(3, 8, true);
    player.speed = 150.0f;
    player.collisionType = Player;
    AddCollisionType(&player, Enemy);
    AddCollisionType(&player, EnemyBullet);

    playerBullets = CreatePlayerBulletArray(MAX_PLAYER_BULLET_POOL, &playerBulletTexture2D);
    enemies = CreateEnemies(MAX_ENEMIES_POOL, &enemy1Texture2D);

    bottomUI.texture = &bottomTexture2D;
    bottomUI.posX = 0;
    bottomUI.posY = GetScreenHeight() - bottomUI.texture->height;

    backgroundMC = LoadMusicStream("assets/audio/background.mp3");
    PlayMusicStream(backgroundMC);
    SetMusicVolume(backgroundMC, 0.25f);
}

void GameLoop()
{  
    while (!WindowShouldClose())
    {
        // INPUT
        Vector2 inputDirection = GetInputDirection();

        if (IsKeyPressed(KEY_SPACE))
        {
            TraceLog(LOG_INFO, "Pressed fire button!");
            GameObject* bullet = GetAvailableGameobject(playerBullets, MAX_PLAYER_BULLET_POOL);
            bullet->pos = player.pos;
            bullet->pos.y -= player.pivotOffset.y;
        }

        if (IsKeyPressed(KEY_E))
        {
            TraceLog(LOG_INFO, "Pressed spawn enemy button!");
            GameObject* enemy = GetAvailableGameobject(enemies, MAX_ENEMIES_POOL);
            enemy->pos.x = GetRandomValue(50, GetScreenWidth() - 50);
            enemy->pos.y = -40.f;
        }

        // UPDATE

        if (IsMusicStreamPlaying(backgroundMC))
        {
            UpdateMusicStream(backgroundMC);
        }

        background1.posY += (int)(GetFrameTime() * SCROLLING_BACKGROUND_SPEED);
        background2.posY += (int)(GetFrameTime() * SCROLLING_BACKGROUND_SPEED);

        if (background1.posY > GetScreenHeight())
        {
            background1.posY -= GetScreenHeight() * 2;
        }

        if (background2.posY > GetScreenHeight())
        {
            background2.posY -= GetScreenHeight() * 2;
        }

        player.velocityNomalize = NomalizeVector2(&inputDirection);
        player.pos.x += (player.velocityNomalize.x * player.speed) * GetFrameTime();
        player.pos.y += (player.velocityNomalize.y * player.speed) * GetFrameTime();

        FixPlayerPosition(&player);

        for (int i = 0; i < MAX_PLAYER_BULLET_POOL; i++)
        {
            if (playerBullets[i].isActive)
            {
                playerBullets[i].pos.y -= playerBullets[i].speed * GetFrameTime();
                if (IsOutOfBounds(&playerBullets[i]))
                {
                    playerBullets[i].isActive = false;
                }
            }
        }

        for (int i = 0; i < MAX_ENEMIES_POOL; i++)
        {
            if (enemies[i].isActive)
            {
                enemies[i].pos.y += enemies[i].speed * GetFrameTime();
                if (IsOutOfBounds(&enemies[i]))
                {
                    enemies[i].isActive = false;
                }
            }
        }

        // ANIMATIONS

        UpdateAnimation2D(&player.anim);

        for (int i = 0; i < MAX_ENEMIES_POOL; i++)
        {
            if (enemies[i].isActive)
            {
                UpdateAnimation2D(&enemies[i].anim);
            }
        }

        // PHYSICS        

        // DRAW

        BeginDrawing();

            ClearBackground(BLACK);

            // Draw Background
            DrawTiledArea(*background1.texture, background1.posX, background1.posY, 20, 15);            
            DrawTiledArea(*background2.texture, background2.posX, background2.posY, 20, 15);
            
            // Draw MiddleBackground

            // Pawn

            DrawTextureRec(*player.texture, (Rectangle){player.anim.currentFrame * player.width, 0, player.width, player.height}, GetPositionWithOffest(&player), WHITE);

            for (int i = 0; i < MAX_PLAYER_BULLET_POOL; i++)
            {
                if (playerBullets[i].isActive)
                {
                    DrawTextureRec(*playerBullets[i].texture, (Rectangle){0, 0, 32, 32}, GetPositionWithOffest(&playerBullets[i]), WHITE);
                }
            }

            for (int i = 0; i < MAX_ENEMIES_POOL; i++)
            {
                if (enemies[i].isActive)
                {
                    DrawTextureRec(*enemies[i].texture, 
                                    (Rectangle){enemies[i].anim.currentFrame * enemies->width, 0, enemies->width, enemies->height},
                                    GetPositionWithOffest(&enemies[i]), 
                                    WHITE);
                }
            }


            DrawCircle((int)(GetScreenWidth() * 0.5f), (int)(GetScreenHeight() * 0.5f), 5, RED);

            // HUD

            DrawTexture(*bottomUI.texture, bottomUI.posX, bottomUI.posY, WHITE);

        EndDrawing();
    }


}

void UnloadGame()
{
    UnloadTexture(backgroundTexture2D);
    UnloadTexture(playerTexture2D);
    UnloadTexture(playerBulletTexture2D);
    UnloadTexture(enemy1Texture2D);
    UnloadTexture(bottomTexture2D);

    UnloadMusicStream(backgroundMC);

    free(playerBullets);
    free(enemies);
}

void ShoutDown()
{
    UnloadGame();
    CloseWindow();
}


void DrawTiledArea(Texture2D texture, int startX, int startY, int tileCountX, int tileCountY)
{
    int tileWidth = texture.width;
    int tileHeight = texture.height;

    for (int y = 0; y < tileCountY; y++)
    {
        for (int x = 0; x < tileCountX; x++)
        {
            int posX = startX + (x * tileWidth);
            int posY = startY + (y * tileHeight);

            DrawTexture(texture, posX, posY, WHITE);
        }
    }
}

Animation2D CreateAnimation2D(int framesCount, int framesSpeed, bool isLooping)
{
    Animation2D anim;
    anim.currentFrame = 0;
    anim.framesCount = framesCount;
    anim.framesCounter = 0;
    anim.framesSpeed = framesSpeed;
    anim.isLooping = isLooping;

    return anim;
}

void UpdateAnimation2D(Animation2D* anim)
{
    anim->framesCounter++;

    if (anim->framesCounter >= (60/anim->framesSpeed))
    {
        anim->framesCounter = 0;
        anim->currentFrame++;

        if (anim->currentFrame > anim->framesCount - 1)
        {
            anim->currentFrame = 0;
        }
    }
}

Vector2 GetInputDirection()
{
    Vector2 inputDirection = (Vector2){0, 0};

    if (IsKeyDown(KEY_W))
    {
        inputDirection.y = -1;
    }
    else if (IsKeyDown(KEY_S))
    {
        inputDirection.y = 1;
    }

    if (IsKeyDown(KEY_A))
    {
        inputDirection.x = -1;
    }
    else if(IsKeyDown(KEY_D))
    {
        inputDirection.x = 1;
    }

    return inputDirection;
}

void AddCollisionType(GameObject* object, CollisionType type)
{
    object->collisionMask |= (unsigned char)type;
}

bool CollisionTypeMatches(GameObject* object, CollisionType type)
{
    return false;
}

Vector2 GetPositionWithOffest(GameObject* object)
{
    return (Vector2){ object->pos.x - object->pivotOffset.x, object->pos.y - object->pivotOffset.y };
}

GameObject* CreatePlayerBulletArray(int size, Texture2D* texture)
{
    GameObject* array = (GameObject*)malloc(size * sizeof(GameObject));

    if (array == NULL)
    {
        TraceLog(LOG_ERROR, "ERROR: memory allocation failed!\n");
        exit(1);
    }

    for (int i = 0; i < size; i++)
    {
        GameObject* gameObject = &array[i];
        gameObject->pivotOffset = (Vector2){16, 16};
        gameObject->isActive = false;
        gameObject->texture = texture;
        gameObject->speed = 500.0f;
        gameObject->collisionType = PlayerBullet;
        AddCollisionType(gameObject, Enemy);
        AddCollisionType(gameObject, EnemyBullet);
    }

    return array;
}

void FixPlayerPosition(GameObject* player)
{
    Vector2 pos_with_offset = GetPositionWithOffest(player);
    
    if (pos_with_offset.y + 10 < 0)
    {
        player->pos.y = player->pivotOffset.y - 10;
    }
    else if(pos_with_offset.y + player->pivotOffset.y + 25 > GetScreenHeight() - 76)
    {
        player->pos.y = ((GetScreenHeight() - 76) - (player->pivotOffset.y - 5));
    }

        if (pos_with_offset.x < 0)
    {
        player->pos.x = player->pivotOffset.x;
    }
    else if(pos_with_offset.x + player->pivotOffset.x + 25 > GetScreenWidth())
    {
        player->pos.x = (GetScreenWidth() - (player->pivotOffset.x - 5));
    }
}

GameObject* GetAvailableGameobject(GameObject* objects, int poolSize)
{
    for (int i = 0; i < poolSize; i++)
    {
        if (objects[i].isActive) continue;

        objects[i].isActive = true;
        return &objects[i];
    }

    return NULL;
}

bool IsOutOfBounds(GameObject* object)
{  
    Vector2 pos_with_offset = GetPositionWithOffest(object);
    if (pos_with_offset.y + object->pivotOffset.y >= -100 && pos_with_offset.y - object->pivotOffset.y <= GetScreenHeight() - 76)
    {
        return false;
    }
    
    return true;
}

GameObject* CreateEnemies(int size, Texture2D* texture)
{
    GameObject* array = (GameObject*)malloc(size * sizeof(GameObject));

    if (array == NULL)
    {
        TraceLog(LOG_ERROR, "ERROR: memory allocation failed!\n");
        exit(1);
    }

    for (int i = 0; i < size; i++)
    {
        GameObject* gameObject = &array[i];
        gameObject->pivotOffset = (Vector2){16, 16};
        gameObject->isActive = false;
        gameObject->texture = texture;
        gameObject->width = texture->width / 3;
        gameObject->height = texture->height;
        gameObject->anim = CreateAnimation2D(3, 8, true);
        gameObject->speed = 100.0f;
        gameObject->collisionType = Enemy;
        AddCollisionType(gameObject, Player);
        AddCollisionType(gameObject, PlayerBullet);
    }

    return array;
}

Vector2 NomalizeVector2(Vector2* vector)
{
    Vector2 vectorNomelaze = (Vector2){0, 0};
    if (vector->x != 0 ||  vector->y != 0)
    {
        const float magnitude = (float)sqrt((vector->x * vector->x) +(vector->y * vector->y));
        vectorNomelaze.x = vector->x / magnitude;
        vectorNomelaze.y = vector->y / magnitude;
    }


    return vectorNomelaze;
}