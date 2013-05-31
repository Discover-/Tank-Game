//#include "game.h"
#include "shareddefines.h"

class Game;

class Player
{
    public:
        Player(Game* _game, float x, float y, SDL_Surface* body, SDL_Surface* pipe);
        ~Player();

        void Update();
        void SetKeysDown(Uint8 index, bool value);
        void HandleTimers(unsigned int diff_time);

        /* MOVEMENT */
        float GetMovingAngle() { return movingAngle; }
        void SetMovingAngle(float val) { movingAngle = val; }
        float GetPosX() { return posX; }
        float GetPosY() { return posY; }
        void SetPosX(float val) { posX = val; }
        void SetPosY(float val) { posY = val; }

        /* MECHANICS */
        void AddBullet(Bullet* bullet) { bullets.push_back(bullet); }
        void AddLandmine(Landmine* landmine) { landmines.push_back(landmine); }
        std::vector<Bullet*> GetBullets() { return bullets; }
        std::vector<Landmine*> GetLandmines() { return landmines; }
        Uint8 GetBulletCount() { return bulletCount; }
        void IncrBulletCount() { bulletCount++; }
        void DecrBulletCount() { bulletCount--; }
        void SetBulletCount(int val) { bulletCount = val; }
        Uint8 GetLandmineCount() { return landmineCount; }
        void IncrLandmineCount() { landmineCount++; }
        void DecrLandmineCount() { landmineCount--; }
        bool CanShoot() { return canShoot; }
        bool CanPlaceLandmine() { return canPlaceLandmine; }
        void SetCanShoot(bool val) { canShoot = val; }
        void SetCanPlaceLandmine(bool val) { canPlaceLandmine = val; }
        void SetShootCooldown(unsigned int val) { shootCooldown = val; }
        void SetPlaceLandmineCooldown(unsigned int val) { landmineCooldown = val; }

        void SetRectPipe(SDL_Rect rect) { rectPipe = rect; }
        void SetRectBody(SDL_Rect rect) { rectBody = rect; }
        void SetRectPipeBody(SDL_Rect rectPipe, SDL_Rect rectBody) { rectPipe = rectPipe; rectBody = rectBody; }
        SDL_Rect GetRectPipe() { return rectPipe; }
        SDL_Rect GetRectBody() { return rectBody; }
        SDL_Surface* GetBodySprite() { return bodySprite; }
        SDL_Surface* GetPipeSprite() { return pipeSprite; }

        float GetMoveSpeed(MoveTypes moveType) { return moveSpeed[moveType]; }

    private:
        Game* game;
        float posX, posY;
        float moveSpeed[MOVE_TYPE_MAX];
        bool keysDown[4];
        //double pipeAngle;
        float movingAngle;
        SDL_Surface* screen;
        Uint8 bulletCount, landmineCount;
        bool canShoot, canPlaceLandmine;
        unsigned int shootCooldown, landmineCooldown;
        std::vector<Bullet*> bullets;
        std::vector<Landmine*> landmines;
        bool inSlowArea;
        SDL_Rect rectPipe, rectBody;
        SDL_Surface* bodySprite, *pipeSprite;
};
