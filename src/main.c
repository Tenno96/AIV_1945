#include "main.h"

#include <stdlib.h>
#include <math.h>

#define MAX_PLAYER_BULLET_POOL 20
#define MAX_ENEMIES_POOL 20
#define MAX_ENEMY_BULLET_POOL 60
#define SCROLLING_BACKGROUND_SPEED 80.0f

#define MAX_PLAYER_ENERGY 100
#define MAX_PLAYER_LIFE 3

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------

static int screenWidth = 640;
static int screenHeight = 480;

static Texture2D backgroundTexture2D;
static Texture2D island1Texture2D;
static Texture2D island2Texture2D;
static Texture2D island3Texture2D;
static Texture2D playerTexture2D;
static Texture2D playerBulletTexture2D;
static Texture2D playerLifeTexture2D;
static Texture2D bottomTexture2D;
static Texture2D enemy1Texture2D;
static Texture2D enemyBulletTexture2D;

static int playerEnergy;
static int playerLife;
static int playerScore;

static GameObject player;
static GameObject* playerBullets;
static GameObject* enemies;
static GameObject* enemyBullets;

static ScrollableObject background1;
static ScrollableObject background2;

static ScrollableObject island;

static TimerHandle spawnEnemy1TimerHandle;
static TimerHandle islandTimerHandle;

static WidgetBar healthBar;

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
    island1Texture2D = LoadTexture("assets/map/island1.png");
    island2Texture2D = LoadTexture("assets/map/island2.png");
    island3Texture2D = LoadTexture("assets/map/island3.png");
    playerTexture2D = LoadTexture("assets/player/myplane_strip3.png");
    playerBulletTexture2D = LoadTexture("assets/player/bullet.png");
    playerLifeTexture2D = LoadTexture("assets/ui/life.png");
    enemy1Texture2D = LoadTexture("assets/enemy/enemy1_strip3.png");
    enemyBulletTexture2D = LoadTexture("assets/enemy/enemybullet1.png");
    bottomTexture2D = LoadTexture("assets/ui/bottom.png");

    background1.texture = &backgroundTexture2D;
    background1.posX = 0;
    background1.posY = 0;

    background2 = background1; 
    background2.posY -= GetScreenHeight();

    ResetIsland();

    islandTimerHandle = (TimerHandle){0.0f, 10.0f, NULL};

    playerEnergy = MAX_PLAYER_ENERGY;
    playerLife = MAX_PLAYER_LIFE;
    playerScore = 0;

    player.isActive = true;
    player.texture = &playerTexture2D;
    player.pos = (Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
    player.pivotOffset = (Vector2){(float)playerTexture2D.height  * 0.5f, (float)playerTexture2D.height * 0.5f};
    player.velocityDir = (Vector2){0, 0};
    player.width = 65;
    player.height = 65;
    player.anim = CreateAnimation2D(3, 8, true);
    player.speed = 150.0f;
    player.collisionType = Player;
    AddCollisionType(&player, Enemy);
    AddCollisionType(&player, EnemyBullet);

    playerBullets = CreatePlayerBulletArray(MAX_PLAYER_BULLET_POOL, &playerBulletTexture2D);
    enemies = CreateEnemies(MAX_ENEMIES_POOL, &enemy1Texture2D);
    enemyBullets = CreateEnemyBulletArray(MAX_ENEMY_BULLET_POOL, &enemyBulletTexture2D);

    spawnEnemy1TimerHandle = (TimerHandle){0.0f, 2.0f, SpawnEnemy};

    healthBar.posX = 11;
    healthBar.posY = GetRenderHeight() - 26.f;
    healthBar.percent = 1.0f;
    healthBar.lengthBar = 128.0f;

    backgroundMC = LoadMusicStream("assets/audio/background.mp3");
    PlayMusicStream(backgroundMC);
    SetMusicVolume(backgroundMC, 0.25f);
}

void GameLoop()
{  
    while (!WindowShouldClose())
    {
        // INPUT
        
        HandlePlayerInput();

        // UPDATE
        
        UpdateGame();

        // PHYSICS

        UpdatePhysics();

        // DRAW

        DrawGame();
    }


}

void HandlePlayerInput()
{
    Vector2 inputDirection = GetInputDirection();
    player.velocityDir = NomalizeVector2(&inputDirection);
    
    if (IsKeyPressed(KEY_SPACE))
    {
        TraceLog(LOG_INFO, "Pressed fire button!");
        GameObject* bullet = GetAvailableGameobject(playerBullets, MAX_PLAYER_BULLET_POOL);
        bullet->pos = player.pos;
        bullet->pos.y -= player.pivotOffset.y;
    }

}

void UpdateGame()
{
      // MUSIC

        // if (IsMusicStreamPlaying(backgroundMC))
        // {
        //     UpdateMusicStream(backgroundMC);
        // }
        
        // BACKGROUND 
        
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

        island.posY += (int)(GetFrameTime() * SCROLLING_BACKGROUND_SPEED);

        islandTimerHandle.timer += GetFrameTime();
        if (islandTimerHandle.timer >= islandTimerHandle.interval)
        {
            islandTimerHandle.timer = 0;
            ResetIsland();
        }

        // PLAYER
        player.pos.x += (player.velocityDir.x * player.speed) * GetFrameTime();
        player.pos.y += (player.velocityDir.y * player.speed) * GetFrameTime();

        FixPlayerPosition(&player);

        // PLAYER_BULLETS, ENEMIES AND ENEMY_BULLETS

        for (int i = 0; i < MAX_PLAYER_BULLET_POOL; i++)
        {
            if (playerBullets[i].isActive)
            {
                playerBullets[i].pos.y += (playerBullets[i].velocityDir.y * playerBullets[i].speed) * GetFrameTime();
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
                enemies[i].pos.y += (enemies->velocityDir.y * enemies[i].speed) * GetFrameTime();

                enemies[i].fireTimerHandle.timer += GetFrameTime();
                if (enemies[i].fireTimerHandle.timer >= enemies[i].fireTimerHandle.interval)
                {
                    enemies[i].fireTimerHandle.timer = 0.0f;
                    enemies[i].fireTimerHandle.TimerCallback(&enemies[i]);
                }
                if (IsOutOfBounds(&enemies[i]))
                {
                    enemies[i].isActive = false;
                }
            }
        }

        for (int i = 0; i < MAX_ENEMY_BULLET_POOL; i++)
        {
            if (enemyBullets[i].isActive)
            {
                enemyBullets[i].pos.y += (enemyBullets[i].velocityDir.y * enemyBullets[i].speed) * GetFrameTime();
                if (IsOutOfBounds(&enemyBullets[i]))
                {
                    enemyBullets[i].isActive = false;
                }
            }
        }

        spawnEnemy1TimerHandle.timer += GetFrameTime();
        if (spawnEnemy1TimerHandle.timer >= spawnEnemy1TimerHandle.interval)
        {
            spawnEnemy1TimerHandle.timer = 0;
            spawnEnemy1TimerHandle.TimerCallback(NULL);
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
}

void UpdatePhysics()
{
        for (int i = 0; i < MAX_ENEMIES_POOL; i++)
        {
            if (enemies[i].isActive)
            {
                Vector2 playerPos = GetPositionWithOffest(&player);
                Vector2 enemyPos = GetPositionWithOffest(&enemies[i]);

                Rectangle playerRec = (Rectangle){playerPos.x, playerPos.y, player.width, player.height};
                Rectangle enemyRect = (Rectangle){enemyPos.x, enemyPos.y, enemies[i].width, enemies[i].height};
                if (CheckCollisionRecs(playerRec, enemyRect))
                {
                    enemies[i].isActive = false;
                    AddScore(10);
                    ReducePlayerEnergy(10);
                    continue;
                }

                for (int k = 0; k < MAX_PLAYER_BULLET_POOL; k++)
                {
                    if (playerBullets[k].isActive)
                    {
                        Vector2 playerBulletPos = GetPositionWithOffest(&playerBullets[k]);

                        Rectangle playerBulletRec = (Rectangle){playerBulletPos.x, playerBulletPos.y, playerBullets[k].width, playerBullets[k].height};
                        if (CheckCollisionRecs(playerBulletRec, enemyRect))
                        {
                            TraceLog(LOG_INFO, "Collisione tra proiettile e nemico!");
                            enemies[i].isActive = false;
                            playerBullets[k].isActive = false;

                            AddScore(10);
                            break;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < MAX_ENEMY_BULLET_POOL; i++)
        {
             if (enemyBullets[i].isActive)
            {
                Vector2 playerPos = GetPositionWithOffest(&player);
                Vector2 enemyBulletPos = GetPositionWithOffest(&enemyBullets[i]);

                Rectangle playerRec = (Rectangle){playerPos.x, playerPos.y, player.width, player.height};
                Rectangle enemyBulletRect = (Rectangle){enemyBulletPos.x, enemyBulletPos.y, enemyBullets[i].width, enemyBullets[i].height};
                if (CheckCollisionRecs(playerRec, enemyBulletRect))
                {
                    enemyBullets[i].isActive = false;
                    ReducePlayerEnergy(10);
                    continue;
                }

                for (int k = 0; k < MAX_PLAYER_BULLET_POOL; k++)
                {
                    if (playerBullets[k].isActive)
                    {
                        Vector2 playerBulletPos = GetPositionWithOffest(&playerBullets[k]);

                        Rectangle playerBulletRec = (Rectangle){playerBulletPos.x, playerBulletPos.y, playerBullets[k].width, playerBullets[k].height};
                        if (CheckCollisionRecs(playerBulletRec, enemyBulletRect))
                        {
                            enemyBullets[i].isActive = false;
                            playerBullets[k].isActive = false;
                            break;
                        }
                    }
                }
            }
        }
}

void DrawGame()
{      
    BeginDrawing();

        ClearBackground(BLACK);
        // Draw Background
        DrawTiledArea(*background1.texture, background1.posX, background1.posY, 20, 15);            
        DrawTiledArea(*background2.texture, background2.posX, background2.posY, 20, 15);
        DrawTexture(*island.texture, island.posX, island.posY, WHITE);
        
        // Draw MiddleBackground
        if (player.isActive)
        {
            DrawTextureRec(*player.texture, (Rectangle){player.anim.currentFrame * player.width, 0, player.width, player.height}, GetPositionWithOffest(&player), WHITE);
        }
        for (int i = 0; i < MAX_PLAYER_BULLET_POOL; i++)
        {
            if (playerBullets[i].isActive)
            {
                DrawTextureRec(*playerBullets[i].texture, 
                                (Rectangle){0, 0, playerBullets[i].width, playerBullets[i].height},
                                GetPositionWithOffest(&playerBullets[i]), WHITE);
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
        for (int i = 0; i < MAX_ENEMY_BULLET_POOL; i++)
        {
            if (enemyBullets[i].isActive)
            {
                DrawTextureRec(*enemyBullets[i].texture, (Rectangle){0, 0, 32, 32}, GetPositionWithOffest(&enemyBullets[i]), WHITE);
            }
        }
        DrawCircle((int)(GetScreenWidth() * 0.5f), (int)(GetScreenHeight() * 0.5f), 5, RED);
        // HUD
        DrawTexture(bottomTexture2D, 0, 404, WHITE);
        DrawLineEx((Vector2){healthBar.posX, healthBar.posY}, (Vector2){healthBar.posX + (healthBar.lengthBar * healthBar.percent), healthBar.posY}, 11.5f, GREEN);
        for (int i = 0; i < playerLife; i++)
        {
            DrawTexture(playerLifeTexture2D, 30 + (30 * i), GetRenderHeight() - 70.f, WHITE);
        }

        DrawText(TextFormat("%d", playerScore), 200, GetRenderHeight() - 40.f, 20, YELLOW);

    EndDrawing();

}

void UnloadGame()
{
    UnloadTexture(backgroundTexture2D);
    UnloadTexture(island1Texture2D);
    UnloadTexture(island2Texture2D);
    UnloadTexture(island3Texture2D);
    UnloadTexture(playerTexture2D);
    UnloadTexture(playerBulletTexture2D);
    UnloadTexture(playerLifeTexture2D);
    UnloadTexture(enemy1Texture2D);
    UnloadTexture(enemyBulletTexture2D);
    UnloadTexture(bottomTexture2D);

    UnloadMusicStream(backgroundMC);

    free(playerBullets);
    free(enemies);
    free(enemyBullets);
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
        gameObject->velocityDir = (Vector2){0, -1};
        gameObject->width = texture->width;
        gameObject->height = texture->height;
        gameObject->isActive = false;
        gameObject->texture = texture;
        gameObject->speed = 500.0f;
        gameObject->collisionType = PlayerBullet;
        AddCollisionType(gameObject, Enemy);
        AddCollisionType(gameObject, EnemyBullet);
    }

    return array;
}

void AddScore(int AddToScore)
{
    playerScore += AddToScore;
}

void ReducePlayerEnergy(int Damage)
{
    playerEnergy -= Damage;

    if (playerEnergy <= 0)
    {
        DecrementPlayerLife();
        playerEnergy = MAX_PLAYER_ENERGY;
    }
    TraceLog(LOG_INFO, "Current player energy: %d", playerEnergy);
    healthBar.percent = (float)((float)playerEnergy / MAX_PLAYER_ENERGY);
}

void DecrementPlayerLife()
{
    playerLife--;

    if (playerLife < 0)
    {
        // TODO: Show HUD GAME OVER
        // TODO: Start delay for restart GAME
    }

    // TODO: Update HUD player life
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
    if (object->velocityDir.y > 0 && pos_with_offset.y - object->pivotOffset.y <= GetScreenHeight() - 76)
    {
        return false;
    }

    if (object->velocityDir.y < 0 && pos_with_offset.y + object->pivotOffset.y >= 0)
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
        gameObject->velocityDir = (Vector2){0, 1};
        gameObject->isActive = false;
        gameObject->texture = texture;
        gameObject->width = texture->width / 3;
        gameObject->height = texture->height;
        gameObject->anim = CreateAnimation2D(3, 8, true);
        gameObject->speed = 100.0f;
        gameObject->collisionType = Enemy;
        AddCollisionType(gameObject, Player);
        AddCollisionType(gameObject, PlayerBullet);
        gameObject->fireTimerHandle.interval = 1.5f;
        gameObject->fireTimerHandle.timer = 0.0f;
        gameObject->fireTimerHandle.TimerCallback = EnemyFire;
    }

    return array;
}

GameObject* CreateEnemyBulletArray(int size, Texture2D* texture)
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
        gameObject->velocityDir = (Vector2){0, 1};
        gameObject->width = texture->width;
        gameObject->height = texture->height;
        gameObject->isActive = false;
        gameObject->texture = texture;
        gameObject->speed = 350.0f;
        gameObject->collisionType = EnemyBullet;
        AddCollisionType(gameObject, PlayerBullet);
        AddCollisionType(gameObject, Player);
    }

    return array;
}

void EnemyFire(void* owner)
{
    GameObject* ownerEnemy = (GameObject*)owner;
    GameObject* bulletEnemy = GetAvailableGameobject(enemyBullets, MAX_ENEMY_BULLET_POOL);
    bulletEnemy->pos.x = ownerEnemy->pos.x;
    bulletEnemy->pos.y = ownerEnemy->pos.y + ownerEnemy->pivotOffset.y;
}

void SpawnEnemy()
{
    TraceLog(LOG_INFO, "Pressed spawn enemy button!");
    GameObject* enemy = GetAvailableGameobject(enemies, MAX_ENEMIES_POOL);
    enemy->pos.x = GetRandomValue(50, GetScreenWidth() - 50);
    enemy->pos.y = -40.f;
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

void ResetIsland()
{
    island.posY = -70;
    island.posX = GetRandomValue(80, GetScreenWidth() - 80);
    int randomIsland = GetRandomValue(0, 2);
    switch (randomIsland)
    {
    case 0:
        island.texture = &island1Texture2D;
        break;

    case 1:
        island.texture = &island2Texture2D;
        break;

    case 2:
        island.texture = &island3Texture2D;
        break;
    }
}