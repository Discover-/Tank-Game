#define SCREEN_WIDTH                1000
#define SCREEN_HEIGHT               600

#define FRAMES_PER_SECOND           30

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

#define COLOR_BLACK                 SDL_MapRGB(screen->format, 0x00, 0x00, 0x00)
#define COLOR_RED                   SDL_MapRGB(screen->format, 0xff, 0x00, 0x00)
#define COLOR_YELLOW                SDL_MapRGB(screen->format, 0xff, 0xff, 0x00)
#define COLOR_WHITE                 SDL_MapRGB(screen->format, 0xff, 0xff, 0xff)
#define COLOR_DARK_BLUE             SDL_MapRGB(screen->format, 0x00, 0x00, 0xff)
#define COLOR_LIGHT_BLUE            SDL_MapRGB(screen->format, 0x00, 0xff, 0xff)
#define COLOR_WHITE2                SDL_MapRGB(screen->format, 0xff, 0xff, 0xff) //! Lijkt een klein beetje donkerder wit
#define COLOR_GREEN                 SDL_MapRGB(screen->format, 0x00, 0xff, 0x00)
#define COLOR_PINK                  SDL_MapRGB(screen->format, 0xff, 0x00, 0xff)

enum MoveSpeed
{
    PLAYER_MOVES_SPEED_FORWARD      = 3,
    PLAYER_MOVES_SPEED_BACKWARD     = 2,
    NPC_MOVES_SPEED_FORWARD         = 2,//2.5,
    NPC_MOVES_SPEED_BACKWARD        = 2, // unused
};

enum MoveTypes
{
    MOVE_TYPE_FORWARD   = 0,
    MOVE_TYPE_BACKWARD  = 1,
};

#define MOVE_TYPE_MAX     2

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

inline bool IsInRange(float currX, float xDest, float currY, float yDest, float distance)
{
    return ((currX < xDest - distance && currX > xDest + distance && currY < yDest - distance && currY > yDest + distance) ||
            (currX > xDest - distance && currX < xDest + distance && currY > yDest - distance && currY < yDest + distance));
}