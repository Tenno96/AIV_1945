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

typedef struct GameObject
{
    bool isActive;
    Vector2 pos;
    Vector2 pivotOffset;
    Vector2 velocityNomalize;
    Texture2D* texture;
    int width;
    int height;
    Animation2D anim;
    float speed;
    unsigned char collisionMask;
    CollisionType collisionType;
}GameObject;

typedef struct PlayerState
{
    int lifeCount;
    int energy;
    int score;
}PlayerState;

typedef struct Widget
{
    int posX;
    int posY;
    Texture2D* texture;
}Widget;



void InitGame();
void GameLoop();
void UnloadGame();
void ShoutDown();


// Draw
void DrawTiledArea(Texture2D texture, int startX, int startY, int tileCountX, int tileCountY);

// Animation
Animation2D CreateAnimation2D(int framesCount, int framesSpeed, bool isLooping);
void UpdateAnimation2D(Animation2D* anim);

// Player 
PlayerState CreatePlayerState(int lifeCount, int energy, int score);
void AddScore(PlayerState* playerState, int scoreToAdd);
void RemoveEnergy(PlayerState* playerState, int quantityToRemove);
void DecrementLifeCount(PlayerState* playerState);
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

// Math
Vector2 NomalizeVector2(Vector2* vector);

#endif // MAIN_H