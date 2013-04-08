//#include "game.h"

class Game;

class Player
{
    public:
        Player(Game* _game, float x, float y);
        ~Player();

        void Update();
        void SetKeysDown(Uint8 index, bool value) { keysDown[index] = value; }
        void HandleTimers(unsigned int diff_time);
        std::vector<Bullet*> GetBullets() { return bullets; }
        std::vector<Landmine*> GetLandmines() { return landmines; }
        Uint8 GetBulletCount() { return bulletCount; }
        void IncrBulletCount() { bulletCount++; }
        void DecrBulletCount() { bulletCount--; }
        Uint8 GetLandmineCount() { return landmineCount; }
        void IncrLandmineCount() { landmineCount++; }
        void DecrLandmineCount() { landmineCount--; }

        /* MOVEMENT */
        float GetMovingAngle() { return movingAngle; }
        void SetMovingAngle(float val) { movingAngle = val; }
        float GetPosX() { return posX; }
        float GetPosY() { return posY; }
        void SetPosX(float val) { posX = val; }
        void SetPosY(float val) { posY = val; }

        /* MECHANICS */
        bool CanShoot() { return canShoot; }
        bool CanPlaceLandmine() { return canPlaceLandmine; }
        void SetCanShoot(bool val) { canShoot = val; }
        void SetCanPlaceLandmine(bool val) { canPlaceLandmine = val; }
        void SetShootCooldown(unsigned int val) { shootCooldown = val; }
        void SetPlaceLandmineCooldown(unsigned int val) { landmineCooldown = val; }

    private:
        Game* game;
        float posX;
        float posY;
        bool keysDown[4];
        //double pipeAngle;
        float movingAngle;
        SDL_Surface* screen;
        Uint8 bulletCount, landmineCount;

        bool canShoot, canPlaceLandmine;
        unsigned int shootCooldown, landmineCooldown;
        std::vector<Bullet*> bullets;
        std::vector<Landmine*> landmines;
};
