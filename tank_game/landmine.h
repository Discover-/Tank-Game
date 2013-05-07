//#include "game.h"

/* Volledige idee:
 - Als er een tank (speler en computer-gestuurde tank) binnen X pixels van een landmijn komt moet hij beginnen met af te tellen. Na
   drie seconden ontploft hij.
 - Alles in een radius zo groot als de explosie moet kapot (tanks, andere mijnen, breakable walls en kogels).
 - 
*/

class Game;

class Landmine
{
    public:
        Landmine(Game* _game, SDL_Surface* _screen, float x, float y);
        ~Landmine();

        void Update();
        void Explode(bool showExplosion = true);
        SDL_Surface* GetSurface() { return timerImage ? imageTimer : imageNormal; }
        SDL_Rect GetRectangle() { return landmineRect; }
        bool IsRemoved() { return isRemoved; }
        float GetPosX() { return posX; }
        float GetPosY() { return posY; }
        void HandleTimers(unsigned int diff_time);

    private:
        SDL_Surface* screen;
        Game* game;
        SDL_Surface* imageNormal;
        SDL_Surface* imageTimer;
        bool isRemoved;
        SDL_Rect landmineRect;
        float posX, posY;
        unsigned int explosionDelay;
        unsigned int timerTillExplode, countdownTenMs;
        bool timerImage, steppedOn;
};
