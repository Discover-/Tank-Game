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
#define COLOR_WHITE2                SDL_MapRGB(screen->format, 0xff, 0xff, 0xff) //! Looks a little dark white
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

enum CollisionSide
{
    SIDE_LEFT           = 0,
    SIDE_RIGHT          = 1,
    SIDE_BOTTOM         = 2,
    SIDE_TOP            = 3,
    SIDE_MAX            = 4,
};

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

struct MineExplosions
{
    float x;
    float y;
    Uint8 frame;
    Uint32 delay;
};

//! A copy of the other default SDL_Rect coming with the SDL library. Required for walls and landmines.
struct SDL_Rect2
{
    SDL_Rect2() : x(0), y(0), w(0), h(0), breakable(false), visible(true), image(NULL) { }
    Sint16 x, y;
    Uint16 w, h;
    bool breakable;
    bool visible;
    SDL_Surface* image;

    //! Return normal SDL_Rect without taking in consideration visible or breakable booleans.
    SDL_Rect GetNormalRect() { SDL_Rect rect = { x, y, w, h }; return rect; }
};

inline int urand(int min, int max)
{
    return (rand() % (max - min + 1) + min);
}

inline bool WillCollision(SDL_Rect rect1, SDL_Rect rect2)
{
    return !(rect1.y >= rect2.y + rect2.h || rect1.x >= rect2.x + rect2.w || rect1.y + rect1.h <= rect2.y || rect1.x + rect1.w <= rect2.x);
}

inline bool WillCollision(SDL_Rect rect1, SDL_Rect2 rect2)
{
    return !(rect1.y >= rect2.y + rect2.h || rect1.x >= rect2.x + rect2.w || rect1.y + rect1.h <= rect2.y || rect1.x + rect1.w <= rect2.x);
}

inline bool WillCollision(SDL_Rect2 rect1, SDL_Rect rect2)
{
    return !(rect1.y >= rect2.y + rect2.h || rect1.x >= rect2.x + rect2.w || rect1.y + rect1.h <= rect2.y || rect1.x + rect1.w <= rect2.x);
}

inline bool WillCollision(SDL_Rect2 rect1, SDL_Rect2 rect2)
{
    return !(rect1.y >= rect2.y + rect2.h || rect1.x >= rect2.x + rect2.w || rect1.y + rect1.h <= rect2.y || rect1.x + rect1.w <= rect2.x);
}

inline bool IsInRange(float currX, float xDest, float currY, float yDest, float distance)
{
    return ((currX < xDest - distance && currX > xDest + distance && currY < yDest - distance && currY > yDest + distance) ||
            (currX > xDest - distance && currX < xDest + distance && currY > yDest - distance && currY < yDest + distance));
}

inline bool HitLeftSide(SDL_Rect rect1, SDL_Rect rect2)
{
    if (WillCollision(rect1, rect2) && rect1.x + rect1.w - 2 <= rect2.x)
        return true;

    return false;
}

inline bool HitRightSide(SDL_Rect rect1, SDL_Rect rect2)
{
    if (WillCollision(rect1, rect2) && rect2.x + rect2.w - 2 <= rect1.x)
        return true;

    return false;
}

inline bool HitBottomSide(SDL_Rect rect1, SDL_Rect rect2)
{
    if (WillCollision(rect1, rect2) && rect1.y + rect1.h -2 >= rect2.y)
        return true;

    return false;
}

inline bool HitTopSide(SDL_Rect rect1, SDL_Rect rect2)
{
    if (WillCollision(rect1, rect2) && rect2.y + rect2.h - 2 >= rect1.y)
        return true;

    return false;
}

//! Returns the side (check enumerator CollisionSide) of rect2 that is currently colliding with rect1. So say rect1 is a bullet and rect2 is a wall and the bullet
//! hits the left side of the wall, it will return SIDE_LEFT. If you, however, switch the parameters so that rect1 is the wall and rect2 is the bullet, the result
//! will (always, in the case for this example) be SIDE_RIGHT (because the right side of the _rotated_ bullet hits the wall).
inline CollisionSide GetSideOfCollision(SDL_Rect rect1, SDL_Rect rect2)
{
    if (!WillCollision(rect1, rect2))
        return SIDE_MAX;

    if (HitLeftSide(rect1, rect2))
        return SIDE_LEFT;

    if (HitRightSide(rect1, rect2))
        return SIDE_RIGHT;

    if (HitBottomSide(rect1, rect2))
        return SIDE_BOTTOM;

    if (HitTopSide(rect1, rect2))
        return SIDE_TOP;

    return SIDE_MAX;
}
