/* BUG COLLECTION:
 * Bullet positioning is off
 * Rotating of enemies spamming
 * 
 * 
 * 
 * 
 * 
 * 
*/

#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <map>
#include <time.h>
#include <unordered_map>
#include <set>
#include "SDL.h"
#include "SDL_rotozoom.h"
#include "bullet.h"
#include "vector2.h"
#include "landmine.h"
#include "animation.h"
#include "SDL_gfxPrimitives.h"
//#include "SDL_net.h"
#include "player.h"
#include "enemy.h"
#include "keyhandler.h"
#include "Windows.h"

//#define main SDL_main
#define SCREEN_WIDTH                1000
#define SCREEN_HEIGHT               600

#define FRAMES_PER_SECOND           30

#define PLAYER_MOVES_SPEED_FORWARD  3
#define PLAYER_MOVES_SPEED_BACKWARD 2
#define NPC_MOVES_SPEED_FORWARD     2.5
#define NPC_MOVES_SPEED_BACKWARD    2 // unused

#define PLAYER_BULLET_LIFES         2
#define PLAYER_BULLET_SPEED_X       5
#define PLAYER_BULLET_SPEED_Y       5

#define PLAYER_MAX_BULLETS          50
#define PLAYER_MAX_LANDMINES        500

#define PLAYER_WIDTH                77
#define PLAYER_HEIGHT               51

#define BULLET_WIDTH                23
#define BULLET_HEIGHT               10

#define LANDMINE_WIDTH              35
#define LANDMINE_HEIGHT             35

#define MAX_MINE_EXPLOSION_FRAMES   17

#define COLOR_BLACK             SDL_MapRGB(screen->format, 0x00, 0x00, 0x00)
#define COLOR_RED               SDL_MapRGB(screen->format, 0xff, 0x00, 0x00)
#define COLOR_YELLOW            SDL_MapRGB(screen->format, 0xff, 0xff, 0x00)
#define COLOR_WHITE             SDL_MapRGB(screen->format, 0xff, 0xff, 0xff)
#define COLOR_DARK_BLUE         SDL_MapRGB(screen->format, 0x00, 0x00, 0xff)
#define COLOR_LIGHT_BLUE        SDL_MapRGB(screen->format, 0x00, 0xff, 0xff)
#define COLOR_WHITE2            SDL_MapRGB(screen->format, 0xff, 0xff, 0xff) //! Lijkt een klein beetje donkerder wit
#define COLOR_GREEN             SDL_MapRGB(screen->format, 0x00, 0xff, 0x00)
#define COLOR_PINK              SDL_MapRGB(screen->format, 0xff, 0x00, 0xff)

struct RGB
{
    //SDL_PixelFormat const* format;
    Uint8 r;
    Uint8 g;
    Uint8 b;
};

struct TemporarilySurfaces
{
    unsigned int despawnTime;
    SDL_Surface* surface;
    char* bmpFile;
    SDL_Rect rect;
    RGB rgb;
};

//! TODO: Dit implementeren...
/*struct RectangleInfo
{
    float x;
    float y;
    float width;
    float height;
    unsigned int color;
};*/

struct MineExplosions
{
    float x;
    float y;
    Uint8 frame;
    Uint32 delay;
};

//! Een zelf-gemaakte kopie van structure SDL_Rect zodat we kunnen weten of een muur kapot te maken is d.m.b. landmijnen.
struct SDL_Rect2
{
    SDL_Rect2() : x(0), y(0), w(0), h(0), breakable(false), visible(true) { }

    Sint16 x, y;
    Uint16 w, h;
    bool breakable;
    bool visible;
};

inline int urand(int min, int max)
{
    return (rand() % (max - min + 1) + min);
}

inline bool WillCollisionAt(SDL_Rect* rect1, SDL_Rect* rect2)
{
    return !(rect1->y >= rect2->y + rect2->h || rect1->x >= rect2->x + rect2->w || rect1->y + rect1->h <= rect2->y || rect1->x + rect1->w <= rect2->x);
}

inline bool WillCollisionAt(SDL_Rect* rect1, SDL_Rect2* rect2)
{
    return !(rect1->y >= rect2->y + rect2->h || rect1->x >= rect2->x + rect2->w || rect1->y + rect1->h <= rect2->y || rect1->x + rect1->w <= rect2->x);
}

inline bool WillCollisionAt(SDL_Rect2* rect1, SDL_Rect* rect2)
{
    return !(rect1->y >= rect2->y + rect2->h || rect1->x >= rect2->x + rect2->w || rect1->y + rect1->h <= rect2->y || rect1->x + rect1->w <= rect2->x);
}

inline bool WillCollisionAt(SDL_Rect2* rect1, SDL_Rect2* rect2)
{
    return !(rect1->y >= rect2->y + rect2->h || rect1->x >= rect2->x + rect2->w || rect1->y + rect1->h <= rect2->y || rect1->x + rect1->w <= rect2->x);
}

class Game
{
    public:
        Game();
        ~Game();

        int Update();
        bool IsRunning() { return isRunning; }
        void ApplySurface(int x, int y, SDL_Surface* source, SDL_Surface* destination);
        void DrawSprite(int srcX, int srcY, int dstX, int dstY, int width, int height, SDL_Surface* source, SDL_Surface* destination);
        void StoreSurfaceByTime(char* bmpFile, SDL_Rect rect, RGB rgb, unsigned int despawnTime);
        void HandleTimers(unsigned int diff_time);
        void UnregistrateBullet(Bullet* bullet);
        void UnregistrateLandmine(Landmine* landmine);
        void AddLandMineExplosion(MineExplosions mineExplosion) { mineExplosions.push_back(mineExplosion); }
        void BlitSurface(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect, RGB rgb);
        SDL_Surface* GetScreen() { return screen; }
        SDL_Event GetEvent() { return _event; }
        std::vector<SDL_Rect2> GetWalls() { return wallRectangles; }
        void SetWalls(std::vector<SDL_Rect2> _wallRectangles) { wallRectangles = _wallRectangles; }
        Player* GetPlayer() { return player; }
        void AddBullet(Bullet* bullet) { allBullets.push_back(bullet); }
        void AddLandmine(Landmine* landmine) { allLandmines.push_back(landmine); }
        std::vector<Bullet*> GetAllBullets() { return allBullets; }
        std::vector<Landmine*> GetAllLandmines() { return allLandmines; }
        double GetPipeAngle() { return pipeAngle; }
        std::vector<Enemy*> GetEnemies() { return enemies; }
        void AddDelayedFunctionCall(Uint8 delayMs, void* function, int param1 = 0, int param2 = 0, int param3 = 0);
        void AddLandmineExplosion();
        void AddWall(Sint16 x, Sint16 y, Sint16 w = 50, Sint16 h = 50, bool breakable = false, bool visible = true);
        void AddWall(SDL_Rect2 wallRect) { wallRectangles.push_back(wallRect); }
        SDL_Rect rotatingRectangle;

    private:
        double pipeAngle;
        bool isRunning;
        SDL_Surface* screen;
        SDL_Event _event;
        std::vector<SDL_Rect2> wallRectangles;
        std::vector<TemporarilySurfaces> temporarilySurfaces;
        std::vector<MineExplosions> mineExplosions;
        SDL_Rect mineExplosionRect[MAX_MINE_EXPLOSION_FRAMES];
        Player* player;
        std::vector<Bullet*> allBullets;
        std::vector<Landmine*> allLandmines;
        std::vector<Enemy*> enemies;

        Uint16 lastMouseX, lastMouseY;
};
