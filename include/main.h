#ifndef MAIN_H
#define MAIN_H

#include "raylib.h"

typedef enum CollisionType
{
    Player = 1,
    Enemy = 2,
    EnemyBullet = 4,
    PlayerBullet = 8
}CollisionType;

typedef struct ScrollableObject
{
    int posX;
    int posY;
    Texture2D* texture;
}ScrollableObject;

typedef struct Animation2D
{
    int framesCount;
    int currentFrame;
    int framesCounter;
    int framesSpeed;
    bool isLooping;
}Animation2D;

typedef struct TimerHandle
{
    float timer;
    float interval;
    void (*TimerCallback)(void*);
}TimerHandle;

typedef struct GameObject
{
    bool isActive;
    Vector2 pos;
    Vector2 pivotOffset;
    Vector2 velocityDir;
    Texture2D* texture;
    int width;
    int height;
    Animation2D anim;
    float speed;
    unsigned char collisionMask;
    CollisionType collisionType;
    TimerHandle fireTimerHandle;
}GameObject;

typedef struct WidgetBar
{
    int posX;
    int posY;
    float percent;
    float lengthBar;
    Texture2D* texture;
}WidgetBar;


void InitGame();
void GameLoop();
void HandlePlayerInput();
void UpdateGame();
void UpdatePhysics();
void DrawGame();
void UnloadGame();
void ShoutDown();


// Draw
void DrawTiledArea(Texture2D texture, int startX, int startY, int tileCountX, int tileCountY);

// Animation
Animation2D CreateAnimation2D(int framesCount, int framesSpeed, bool isLooping);
void UpdateAnimation2D(Animation2D* anim);

// Player 
void AddScore(int AddToScore);
void ReducePlayerEnergy(int Damage);
void DecrementPlayerLife();
void FixPlayerPosition(GameObject* player);

Vector2 GetInputDirection();

// Physic
void AddCollisionType(GameObject* object, CollisionType type);
bool CollisionTypeMatches(GameObject* object, CollisionType type);

void PlayerFire();

void SetPosition(GameObject* object, Vector2 newPos);
Vector2 GetPositionWithOffest(GameObject* object);
GameObject* CreatePlayerBulletArray(int size, Texture2D* texture);
GameObject* GetAvailableGameobject(GameObject* objects, int poolSize);
bool IsOutOfBounds(GameObject* object);


// Enemy

GameObject* CreateEnemies(int size, Texture2D* texture);
GameObject* CreateEnemyBulletArray(int size, Texture2D* texture);
void EnemyFire(void* owner);
void SpawnEnemy();

// Math
Vector2 NomalizeVector2(Vector2* vector);

void ResetIsland();

#endif // MAIN_H