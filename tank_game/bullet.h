//#include "game.h"

class Game;
class Enemy;

class Bullet
{
    public:
        Bullet(Game* _game, SDL_Surface* _screen, float x, float y, int w, int h, int xVel, int yVel, int _life, double _pipeAngle, bool shooterIsPlr = true);
        ~Bullet();

        void Update();
        void Explode(bool showExplosion = true);
        SDL_Surface* GetSurface() { return image; }
        void SetRemainingLife(unsigned int _life) { life = _life; }
        SDL_Rect GetRectangle() { return bulletRect; }
        bool IsRemoved() { return isRemoved; }
        float GetPosX() { return x; }
        float GetPosY() { return y; }

    private:
        SDL_Surface* screen;
        Game* game;
        int xVelocity, yVelocity;
        SDL_Surface* image;
        bool isRemoved;
        SDL_Rect bulletRect;
        unsigned int life;
        double directionAngle;
        double rotateAngle;
        float x, y;
        SDL_Surface* rotatedBullet;
        bool shooterIsPlr;
};
