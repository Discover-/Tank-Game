//#include "game.h"

/* Volledige idee:
 - Als er een tank (speler en computer-gestuurde tank) binnen X pixels van een landmijn komt moet hij beginnen met af te tellen. Na
   drie seconden ontploft hij.
 - Alles in een radius zo groot als de explosie moet kapot (tanks, andere mijnen, breakable walls en kogels).
 - 
*/

class Game;
class Vector2;

class Landmine
{
    public:
        Landmine(Game* _game, SDL_Surface* _screen, SDL_Surface* img, float x, float y, int w, int h);
        ~Landmine();
        void Update();
        void Explode();
        SDL_Surface* GetSurface() { return image; }
        SDL_Rect GetRectangle() { return landmineRect; }
        Sint16 GetX() { return landmineRect.x; }
        Sint16 GetY() { return landmineRect.y; }
        //Vector2 coords;

    private:
        SDL_Surface* screen;
        Game* game;
        SDL_Surface* image;
        bool isRemoved;
        SDL_Rect landmineRect;
};
