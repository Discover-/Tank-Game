//#include "game.h"


struct WaypointNode
{
    Uint32 pointId;
    float x;
    float y;
};

struct WaypointInformation
{
    std::vector<WaypointNode> nodes;
    Uint32 currDestPointId;
    float xVelocity;
    float yVelocity;
    bool repeat;
    bool repeatReversed;
    bool pickRandomAtEnd;
};

class Game;

class Enemy
{
    public:
        Enemy(Game* _game, float x, float y, SDL_Surface* body, SDL_Surface* pipe, SDL_Rect _rectBodyPipe, EnemyTypes = ENEMY_TYPE_TIER_ONE);
        ~Enemy();

        void Update();
        void HandleTimers(unsigned int diff_time);
        std::vector<Bullet*> GetBullets() { return bullets; }
        std::vector<Landmine*> GetLandmines() { return landmines; }

        /* MOVEMENT */
        float GetMovingAngle() { return movingAngle; }
        void SetMovingAngle(float val) { movingAngle = val; }
        float GetPosX() { return posX; }
        float GetPosY() { return posY; }
        void SetPosX(float val) { posX = val; }
        void SetPosY(float val) { posY = val; }
        void SetRectPosX(Sint16 val, bool body, bool pipe);
        void SetRectPosY(Sint16 val, bool body, bool pipe);
        float GetStartPosX() { return startPosX; }
        float GetStartPosY() { return startPosY; }

        void SetRotatedInfo(SDL_Surface* body, SDL_Surface* pipe, SDL_Surface* _bodySprite, SDL_Surface* _pipeSprite) { rotatedBody = body; rotatedPipe = pipe; bodySprite = _bodySprite; pipeSprite = _pipeSprite; }
        void SetRotatedBodySurface(SDL_Surface* body) { rotatedBody = body; }
        void SetRotatedPipeSurface(SDL_Surface* pipe) { rotatedPipe = pipe; }
        SDL_Surface* GetRotatedBodySurface() { return rotatedBody; }
        SDL_Surface* GetRotatedPipeSurface() { return rotatedPipe; }
        SDL_Surface* GetBodySprite() { return bodySprite; }
        SDL_Surface* GetPipeSprite() { return pipeSprite; }

        void SetRotatedBodyRect(SDL_Rect body) { rectBody = body; }
        void SetRotatedPipeRect(SDL_Rect pipe) { rectPipe = pipe; }
        SDL_Rect GetRotatedBodyRect() { return rectBody; }
        SDL_Rect GetRotatedPipeRect() { return rectPipe; }

        float GetRotatingPipeAngle() { return rotatingPipeAngle; }
        void SetRotatingPipeAngle(float val) { rotatingPipeAngle = val; }
        float GetRotatingBodyAngle() { return rotatingBodyAngle; }
        void SetRotatingBodyAngle(float val) { rotatingBodyAngle = val; }

        /* MECHANICS */
        bool CanShoot() { return canShoot; }
        bool CanPlaceLandmine() { return canPlaceLandmine; }
        void SetCanShoot(bool val) { canShoot = val; }
        void SetCanPlaceLandmine(bool val) { canPlaceLandmine = val; }
        void SetShootCooldown(unsigned int val) { shootCooldown = val; }
        void SetPlaceLandmineCooldown(unsigned int val) { landmineCooldown = val; }

        /* WAYPOINTS */
        void InitializeWaypoints(bool eraseCurrent = false);
        std::vector<WaypointInformation>& GetWaypoints() { return waypoints; }

        void JustDied();
        bool IsAlive() { return !isDead; }
        void SetIsAlive(bool val) { isDead = !val; }

        Uint8 GetBulletCount() { return bulletCount; }
        void IncrBulletCount() { bulletCount++; }
        void DecrBulletCount() { bulletCount--; }
        void SetBulletCount(int val) { bulletCount = val; }

        float GetMoveSpeed(MoveTypes moveType) { return moveSpeed[moveType]; }

    private:
        SDL_Surface* screen;
        Game* game;
        float posX, posY;
        float startPosX, startPosY;
        //double pipeAngle;
        float movingAngle;
        float rotatingBodyAngle, rotatingPipeAngle;
        SDL_Surface* bodySprite;
        SDL_Surface* pipeSprite;
        SDL_Surface* rotatedBody;
        SDL_Surface* rotatedPipe;
        SDL_Rect rectBody;
        SDL_Rect rectPipe;

        bool canShoot, canPlaceLandmine;
        unsigned int shootCooldown, landmineCooldown;
        std::vector<Bullet*> bullets;
        std::vector<Landmine*> landmines;

        std::vector<WaypointInformation> waypoints;

        Uint32 lastPointIncreaseTime;

        bool isDead;

        Uint32 randomShootTimer;

        float moveSpeed[MOVE_TYPE_MAX];
        bool inSlowArea;

        Uint8 bulletCount;

        EnemyTypes enemyType;
};
