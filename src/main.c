#include "main.h"

#include <stdlib.h>
#include <math.h>

#define MAX_PLAYER_BULLET_POOL 20
#define MAX_ENEMIES_POOL 20
#define MAX_ENEMY_BULLET_POOL 60
#define MAX_ENEMY_EXPLOSION_POOL 10

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
static Texture2D playerExplosionTexture2D;
static Texture2D playerBulletTexture2D;
static Texture2D playerLifeTexture2D;

static Texture2D bottomTexture2D;

static Texture2D enemy1Texture2D;
static Texture2D enemyBulletTexture2D;
static Texture2D enemyExplosionTexture2D;

static int playerEnergy;
static int playerLife;
static int playerScore;

static GameObject player;
static GameObject playerExplosion;
static aiv_vector_t playerBullets;

static aiv_vector_t enemies;
static aiv_vector_t enemyBullets;
static aiv_vector_t enemyExplosions;


static ScrollableObject background1;
static ScrollableObject background2;

static ScrollableObject island;

static TimerHandle spawnEnemy1TimerHandle;
static TimerHandle islandTimerHandle;

static WidgetBar healthBar;

static Music backgroundMC;
static Sound playerExplosionSFX;
static Sound enemyExplosionSFX;


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
    playerExplosionTexture2D = LoadTexture("assets/player/explosion2_strip7.png");
    playerBulletTexture2D = LoadTexture("assets/player/bullet.png");
    playerLifeTexture2D = LoadTexture("assets/ui/life.png");

    enemy1Texture2D = LoadTexture("assets/enemy/enemy1_strip3.png");
    enemyBulletTexture2D = LoadTexture("assets/enemy/enemybullet1.png");
    enemyExplosionTexture2D = LoadTexture("assets/enemy/explosion1_strip6.png");

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

    player.texture = &playerTexture2D;
    player.pivotOffset = (Vector2){(float)playerTexture2D.height  * 0.5f, (float)playerTexture2D.height * 0.5f};
    player.velocityDir = (Vector2){0, 0};
    player.width = 65;
    player.height = 65;
    player.anim = CreateAnimation2D(3, 8, true);
    player.speed = 200.0f;
    // player.collisionType = E_Player;
    // AddCollisionType(&player, E_Enemy);
    // AddCollisionType(&player, E_EnemyBullet);
    RespawnPlayer();

    playerExplosion.isActive = false;
    playerExplosion.texture = &playerExplosionTexture2D;
    playerExplosion.width = playerExplosionTexture2D.height;
    playerExplosion.height = playerExplosionTexture2D.height;
    playerExplosion.pivotOffset = (Vector2){playerExplosion.width * 0.5f, playerExplosion.height * 0.5f};
    playerExplosion.anim = CreateAnimation2D(playerExplosionTexture2D.width / playerExplosionTexture2D.height, 8, false); 

    CreatePlayerBulletArray();
    CreateEnemies();
    CreateEnemyBulletArray();
    CreateEnemyExplosionArray();

    spawnEnemy1TimerHandle = (TimerHandle){0.0f, 2.0f, SpawnEnemy};

    healthBar.posX = 11;
    healthBar.posY = GetRenderHeight() - 26.f;
    healthBar.percent = 1.0f;
    healthBar.lengthBar = 128.0f;

    backgroundMC = LoadMusicStream("assets/audio/background.mp3");
    PlayMusicStream(backgroundMC);
    SetMusicVolume(backgroundMC, 0.25f);

    playerExplosionSFX = LoadSound("assets/audio/snd_explosion2.wav");
    SetSoundVolume(playerExplosionSFX, 0.25f);

    enemyExplosionSFX = LoadSound("assets/audio/snd_explosion1.wav");
    SetSoundVolume(enemyExplosionSFX, 0.25f);
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
        GameObject* bullet =  GetAvailableGameobject(&playerBullets);
        bullet->isActive = true;
        bullet->pos = player.pos;
        bullet->pos.y -= player.pivotOffset.y;
    }

}

void UpdateGame()
{
      // MUSIC

        if (IsMusicStreamPlaying(backgroundMC))
        {
            UpdateMusicStream(backgroundMC);
        }
        
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
        UpdateAnimation2D(&player.anim);

        if (!player.isActive)
        {
            bool isFinished = UpdateAnimation2D(&playerExplosion.anim);

            if (isFinished)
            {
                playerExplosion.isActive = false;
                RespawnPlayer();
            }
        }

        // PLAYER_BULLETS, ENEMIES AND ENEMY_BULLETS

        for (int i = 0; i < MAX_PLAYER_BULLET_POOL; i++)
        {
            GameObject* playerBullet = (GameObject*)playerBullets.items[i];
            if (playerBullet->isActive)
            {
                playerBullet->pos.y += (playerBullet->velocityDir.y * playerBullet->speed) * GetFrameTime();
                if (IsOutOfBounds(playerBullet))
                {
                    playerBullet->isActive = false;
                }
            }
        }

        for (int i = 0; i < MAX_ENEMIES_POOL; i++)
        {
            GameObject* enemy = aiv_vector_at(&enemies, i);
            if (enemy->isActive)
            {
                enemy->pos.y += (enemy->velocityDir.y * enemy->speed) * GetFrameTime();
                UpdateAnimation2D(&enemy->anim);

                enemy->fireTimerHandle.timer += GetFrameTime();
                if (enemy->fireTimerHandle.timer >= enemy->fireTimerHandle.interval)
                {
                    enemy->fireTimerHandle.timer = 0.0f;
                    enemy->fireTimerHandle.TimerCallback(enemy);
                }
                if (IsOutOfBounds(enemy))
                {
                    enemy->isActive = false;
                }
            }
        }

        for (int i = 0; i < MAX_ENEMY_BULLET_POOL; i++)
        {
            GameObject* enemyBullet = aiv_vector_at(&enemyBullets, i);
            if (enemyBullet->isActive)
            {
                enemyBullet->pos.y += (enemyBullet->velocityDir.y * enemyBullet->speed) * GetFrameTime();
                if (IsOutOfBounds(enemyBullet))
                {
                    enemyBullet->isActive = false;
                }
            }
        }

        for (int i = 0; i < MAX_ENEMY_EXPLOSION_POOL; i++)
        {
            GameObject* enemyExplosion = aiv_vector_at(&enemyExplosions, i);
            if (enemyExplosion->isActive)
            {
                bool isFinished = UpdateAnimation2D(&enemyExplosion->anim);
                if (isFinished)
                {
                    enemyExplosion->isActive = false;
                }
            }
        }

        spawnEnemy1TimerHandle.timer += GetFrameTime();
        if (spawnEnemy1TimerHandle.timer >= spawnEnemy1TimerHandle.interval)
        {
            spawnEnemy1TimerHandle.interval = GetRandomFloatValue(2.5f, 4.0f);
            spawnEnemy1TimerHandle.timer = 0;
            spawnEnemy1TimerHandle.TimerCallback(NULL);
        }
}

void UpdatePhysics()
{
        for (int i = 0; i < MAX_ENEMIES_POOL; i++)
        {
            GameObject* enemy = aiv_vector_at(&enemies, i);
            if (enemy->isActive)
            {
                Vector2 playerPos = GetPositionWithOffest(&player);
                Vector2 enemyPos = GetPositionWithOffest(enemy);

                Rectangle playerRec = (Rectangle){playerPos.x, playerPos.y, player.width, player.height};
                Rectangle enemyRect = (Rectangle){enemyPos.x, enemyPos.y, enemy->width, enemy->height};
                if (CheckCollisionRecs(playerRec, enemyRect))
                {
                    enemy->isActive = false;
                    PlayEnemyExplosion(enemy);
                    AddScore(10);
                    ReducePlayerEnergy(10);
                    continue;
                }

                for (int k = 0; k < MAX_PLAYER_BULLET_POOL; k++)
                {
                    GameObject* playerBullet = aiv_vector_at(&playerBullets, k);

                    if (playerBullet->isActive)
                    {
                        Vector2 playerBulletPos = GetPositionWithOffest(playerBullet);

                        Rectangle playerBulletRec = (Rectangle){playerBulletPos.x, playerBulletPos.y, playerBullet->width, playerBullet->height};
                        if (CheckCollisionRecs(playerBulletRec, enemyRect))
                        {
                            enemy->isActive = false;
                            PlayEnemyExplosion(enemy);
                            playerBullet->isActive = false;

                            AddScore(10);
                            break;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < MAX_ENEMY_BULLET_POOL; i++)
        {
            GameObject* enemyBullet = aiv_vector_at(&enemyBullets, i);
             if (enemyBullet->isActive)
            {
                Vector2 playerPos = GetPositionWithOffest(&player);
                Vector2 enemyBulletPos = GetPositionWithOffest(enemyBullet);

                Rectangle playerRec = (Rectangle){playerPos.x, playerPos.y, player.width, player.height};
                Rectangle enemyBulletRect = (Rectangle){enemyBulletPos.x, enemyBulletPos.y, enemyBullet->width, enemyBullet->height};
                if (CheckCollisionRecs(playerRec, enemyBulletRect))
                {
                    enemyBullet->isActive = false;
                    ReducePlayerEnergy(10);
                    continue;
                }

                for (int k = 0; k < MAX_PLAYER_BULLET_POOL; k++)
                {
                    GameObject* playerBullet = aiv_vector_at(&playerBullets, k);

                    if (playerBullet->isActive)
                    {
                        Vector2 playerBulletPos = GetPositionWithOffest(playerBullet);

                        Rectangle playerBulletRec = (Rectangle){playerBulletPos.x, playerBulletPos.y, playerBullet->width, playerBullet->height};
                        if (CheckCollisionRecs(playerBulletRec, enemyBulletRect))
                        {
                            enemyBullet->isActive = false;
                            playerBullet->isActive = false;
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

        if (playerExplosion.isActive)
        {
            DrawTextureRec(*playerExplosion.texture,
                            (Rectangle){playerExplosion.anim.currentFrame * playerExplosion.width, 0, playerExplosion.width, playerExplosion.height},
                            GetPositionWithOffest(&playerExplosion), WHITE);
        }

        for (int i = 0; i < MAX_PLAYER_BULLET_POOL; i++)
        {
            GameObject* playerBullet = (GameObject*)playerBullets.items[i];
            
            if (playerBullet->isActive)
            {
                DrawTextureRec(*playerBullet->texture, 
                                (Rectangle){0, 0, playerBullet->width, playerBullet->height},
                                GetPositionWithOffest(playerBullet), WHITE);
            }
        }

        for (int i = 0; i < MAX_ENEMIES_POOL; i++)
        {
             GameObject* enemy = aiv_vector_at(&enemies, i);
            if (enemy->isActive)
            {
                DrawTextureRec(*enemy->texture, 
                                (Rectangle){enemy->anim.currentFrame * enemy->width, 0, enemy->width, enemy->height},
                                GetPositionWithOffest(enemy), 
                                WHITE);
            }
        }

        for (int i = 0; i < MAX_ENEMY_BULLET_POOL; i++)
        {
            GameObject* enemyBullet = aiv_vector_at(&enemyBullets, i);
            if (enemyBullet->isActive)
            {
                DrawTextureRec(*enemyBullet->texture, (Rectangle){0, 0, enemyBullet->width, enemyBullet->height}, GetPositionWithOffest(enemyBullet), WHITE);
            }
        }

        for (int i = 0; i < MAX_ENEMY_EXPLOSION_POOL; i++)
        {
            GameObject* enemyExplosion = aiv_vector_at(&enemyExplosions, i);
            if (enemyExplosion->isActive)
            {
                DrawTextureRec(*enemyExplosion->texture, 
                                (Rectangle){enemyExplosion->anim.currentFrame * enemyExplosion->width, 0, enemyExplosion->width, enemyExplosion->height}, 
                                GetPositionWithOffest(enemyExplosion), 
                                WHITE);
            }
        }
        
        // DrawCircle((int)(GetScreenWidth() * 0.5f), (int)(GetScreenHeight() * 0.5f), 5, RED);

        // HUD
        DrawTexture(bottomTexture2D, 0, 404, WHITE);
        DrawLineEx((Vector2){healthBar.posX, healthBar.posY}, (Vector2){healthBar.posX + (healthBar.lengthBar * healthBar.percent), healthBar.posY}, 11.5f, GREEN);
        for (int i = 0; i < playerLife; i++)
        {
            DrawTexture(playerLifeTexture2D, 30 + (30 * i), GetRenderHeight() - 70.f, WHITE);
        }

        DrawText(TextFormat("%04d", playerScore), 180, GetRenderHeight() - 40.f, 20, YELLOW);

    EndDrawing();

}

void UnloadGame()
{
    UnloadTexture(backgroundTexture2D);
    UnloadTexture(island1Texture2D);
    UnloadTexture(island2Texture2D);
    UnloadTexture(island3Texture2D);
    UnloadTexture(playerTexture2D);
    UnloadTexture(playerExplosionTexture2D);
    UnloadTexture(playerBulletTexture2D);
    UnloadTexture(playerLifeTexture2D);
    UnloadTexture(enemy1Texture2D);
    UnloadTexture(enemyBulletTexture2D);
    UnloadTexture(enemyExplosionTexture2D);
    UnloadTexture(bottomTexture2D);

    UnloadMusicStream(backgroundMC);

    UnloadSound(playerExplosionSFX);
    UnloadSound(enemyExplosionSFX);

    aiv_vector_destroy(&playerBullets);
    aiv_vector_destroy(&enemies);
    aiv_vector_destroy(&enemyBullets);
    aiv_vector_destroy(&enemyExplosions);
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

bool UpdateAnimation2D(Animation2D* anim)
{
    anim->framesCounter++;

    if (anim->framesCounter >= (60/anim->framesSpeed))
    {
        anim->framesCounter = 0;
        anim->currentFrame++;

        if (anim->currentFrame > anim->framesCount - 1)
        {
            if (anim->isLooping)
            {
                anim->currentFrame = 0;
            }
            else
            {
                anim->currentFrame = -1;
                return true;
            }
        }
    }
    
    return false;
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

// void AddCollisionType(GameObject* object, CollisionType type)
// {
//     object->collisionMask |= (unsigned char)type;
// }

// bool CollisionTypeMatches(GameObject* object, CollisionType type)
// {
//     return false;
// }

Vector2 GetPositionWithOffest(GameObject* object)
{
    return (Vector2){ object->pos.x - object->pivotOffset.x, object->pos.y - object->pivotOffset.y };
}

void CreatePlayerBulletArray()
{
    playerBullets = aiv_vector_new();

    for (int i = 0; i < MAX_PLAYER_BULLET_POOL; i++)
    {
        GameObject* gameObject = (GameObject*)malloc(sizeof(GameObject));
        gameObject->pivotOffset = (Vector2){playerBulletTexture2D.width * 0.5f, playerBulletTexture2D.height * 0.5f};
        gameObject->velocityDir = (Vector2){0, -1};
        gameObject->width = playerBulletTexture2D.width;
        gameObject->height = playerBulletTexture2D.height;
        gameObject->isActive = false;
        gameObject->texture = &playerBulletTexture2D;
        gameObject->speed = 500.0f;
        // gameObject->collisionType = E_PlayerBullet;
        // AddCollisionType(gameObject, E_Enemy);
        // AddCollisionType(gameObject, E_EnemyBullet);

        aiv_vector_add(&playerBullets, gameObject);
    }
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
        PlayPlayerExplosion();
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
        TraceLog(LOG_INFO, "GAME OVER!");
    }
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

void RespawnPlayer()
{
    player.pos = (Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
    player.isActive = true;
}

void PlayPlayerExplosion()
{
    player.isActive = false;

    playerExplosion.pos = player.pos;
    playerExplosion.anim.currentFrame = 0;
    playerExplosion.isActive = true;

    PlaySound(playerExplosionSFX);
}

GameObject* GetAvailableGameobject(aiv_vector_t* vector)
{
    for (int i = 0; i < vector->count; i++)
    {
        GameObject* object = aiv_vector_at(vector, i);
        if (object->isActive) continue;

        object->isActive = true;
        return aiv_vector_at(vector, i);
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

void CreateEnemies()
{
    enemies = aiv_vector_new();

    for (int i = 0; i < MAX_ENEMIES_POOL; i++)
    {
        GameObject* gameObject = malloc(sizeof(GameObject));
        gameObject->pivotOffset = (Vector2){16, 16};
        gameObject->velocityDir = (Vector2){0, 1};
        gameObject->isActive = false;
        gameObject->texture = &enemy1Texture2D;
        gameObject->width = enemy1Texture2D.width / 3;
        gameObject->height = enemy1Texture2D.height;
        gameObject->anim = CreateAnimation2D(3, 8, true);
        gameObject->speed = 100.0f;
        // gameObject->collisionType = E_Enemy;
        // AddCollisionType(gameObject, E_Player);
        // AddCollisionType(gameObject, E_PlayerBullet);
        gameObject->fireTimerHandle.interval = GetRandomFloatValue(1.7f, 3.7f);
        gameObject->fireTimerHandle.timer = 0.0f;
        gameObject->fireTimerHandle.TimerCallback = EnemyFire;

        aiv_vector_add(&enemies, gameObject);
    }
}

void CreateEnemyBulletArray()
{
    enemyBullets = aiv_vector_new();
    
    for (int i = 0; i < MAX_ENEMY_BULLET_POOL; i++)
    {
        GameObject* gameObject = malloc(sizeof(GameObject));
        gameObject->pivotOffset = (Vector2){enemyBulletTexture2D.width * 0.5f, enemyBulletTexture2D.height * 0.5f};
        gameObject->velocityDir = (Vector2){0, 1};
        gameObject->width = enemyBulletTexture2D.width;
        gameObject->height = enemyBulletTexture2D.height;
        gameObject->isActive = false;
        gameObject->texture = &enemyBulletTexture2D;
        gameObject->speed = 350.0f;
        // gameObject->collisionType = E_EnemyBullet;
        // AddCollisionType(gameObject, E_PlayerBullet);
        // AddCollisionType(gameObject, E_Player);

        aiv_vector_add(&enemyBullets, gameObject);
    }
}

void CreateEnemyExplosionArray()
{
    enemyExplosions = aiv_vector_new();

    for (int i = 0; i < MAX_ENEMY_EXPLOSION_POOL; i++)
    {
        GameObject* gameObject = malloc(sizeof(GameObject));
        gameObject->isActive = false;
        gameObject->texture = &enemyExplosionTexture2D;
        gameObject->width = enemyExplosionTexture2D.height;
        gameObject->height = enemyExplosionTexture2D.height;
        gameObject->pivotOffset = (Vector2){gameObject->width * 0.5f, gameObject->height * 0.5f};
        gameObject->anim = CreateAnimation2D(enemyExplosionTexture2D.width / enemyExplosionTexture2D.height, 8, false);

        aiv_vector_add(&enemyExplosions, gameObject);
    }
}

void EnemyFire(void* owner)
{
    GameObject* ownerEnemy = (GameObject*)owner;
    GameObject* bulletEnemy = GetAvailableGameobject(&enemyBullets);
    bulletEnemy->pos.x = ownerEnemy->pos.x;
    bulletEnemy->pos.y = ownerEnemy->pos.y + ownerEnemy->pivotOffset.y;
}

void SpawnEnemy()
{
    int startX = GetRandomValue(50, GetScreenWidth() - 50);
    int startY = -40.f;
    int spawnAmount = GetRandomValue(2, 4);
    bool spawnToLeft = startX > GetScreenWidth() * 0.5f;
    int directionMultiplier = spawnToLeft ? -1 : 1;

    for (int i = 0; i < spawnAmount; i++)
    {
        GameObject* enemy = GetAvailableGameobject(&enemies);
        float xOffset = (enemy->width + 5) * i;
        float yOffset = -enemy->height * i;
        enemy->pos.x = startX + (xOffset * directionMultiplier);
        enemy->pos.y = startY + yOffset;
    }
}

void PlayEnemyExplosion(GameObject* owner)
{
    GameObject* enemyExplosion = GetAvailableGameobject(&enemyExplosions);

    enemyExplosion->pos = owner->pos;
    enemyExplosion->anim.currentFrame = 0;
    enemyExplosion->isActive = true;

    PlaySound(enemyExplosionSFX);
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

float GetRandomFloatValue(float min, float max)
{
    float r = (float)rand()/(float)RAND_MAX;
    return r * (max - min) + min;
}