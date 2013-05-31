//#include "game.h"

class Game;
class Enemy;

class Bullet
{
    public:
        Bullet(Game* _game, SDL_Surface* _screen, float x, float y, double _pipeAngle, bool shooterIsPlr = true, int life = 2, float xVel = 5, float yVel = 5, bool followPlayer = false);
        ~Bullet();

        void Update();
        void Explode(bool showExplosion = true);
        SDL_Surface* GetSurface() { return image; }
        void SetRemainingLife(unsigned int _life) { life = _life; }
        unsigned int GetRemainingLife() { return life; }

        SDL_Rect GetRectangle() { return bulletRect; }
        bool IsRemoved() { return isRemoved; }
        float GetPosX() { return posX; }
        float GetPosY() { return posY; }
        void SetVelocityX(float val) { xVelocity = val; }
        void SetVelocityY(float val) { yVelocity = val; }
        float GetVelocityX() { return xVelocity; }
        float GetVelocityY() { return yVelocity; }

    private:
        SDL_Surface* screen;
        Game* game;
        float xVelocity, yVelocity;
        SDL_Surface* image;
        bool isRemoved;
        SDL_Rect bulletRect;
        unsigned int life;
        double directionAngle;
        double rotateAngle;
        float posX, posY;
        SDL_Surface* rotatedBullet;
        bool shooterIsPlr;
        bool inSlowArea;
        bool followPlayer;
};
