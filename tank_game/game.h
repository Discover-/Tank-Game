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
#include <string>
#include "Windows.h"
#include "SDL.h"
#include "SDL_rotozoom.h"
#include "SDL_gfxPrimitives.h"
#include "bullet.h"
#include "landmine.h"
//#include "SDL_net.h"
#include "player.h"
#include "enemy.h"
#include "keyhandler.h"

//#define main SDL_main

class Game
{
    public:
        Game();
        ~Game();

        int Update();
        bool IsRunning() { return isRunning; }
        void StoreSurfaceByTime(char* bmpFile, SDL_Rect rect, RGB rgb, unsigned int despawnTime, SDL_Surface* image = NULL);
        void HandleTimers(unsigned int diff_time);
        void UnregistrateBullet(Bullet* bullet);
        void UnregistrateLandmine(Landmine* landmine);
        void AddGrowingExplosion(GrowingExplosions mineExplosion) { growingExplosions.push_back(mineExplosion); }
        void AddGrowingExplosion(float x, float y, Uint8 frame, Uint32 delay, Uint32 maxFrames) { GrowingExplosions mineExpl = { Sint16(x), Sint16(y), frame, delay, maxFrames }; AddGrowingExplosion(mineExpl); }
        void BlitSurface(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect, RGB rgb);
        SDL_Surface* GetScreen() { return screen; }
        SDL_Event GetEvent() { return _event; }
        std::vector<SDL_Rect2>& GetWalls() { return wallRectangles; }
        std::vector<SDL_Rect2>& GetMergedWalls() { return mergedWalls; }
        Player* GetPlayer() { return player; }
        void AddBullet(Bullet* bullet) { allBullets.push_back(bullet); }
        void AddLandmine(Landmine* landmine) { allLandmines.push_back(landmine); }
        void RemoveBullet(Bullet* bullet);
        void RemoveLandmine(Landmine* landmine);
        std::vector<Bullet*> GetAllBullets() { return allBullets; }
        std::vector<Landmine*> GetAllLandmines() { return allLandmines; }
        double GetPipeAngle() { return pipeAngle; }
        std::vector<Enemy*> GetEnemies() { return enemies; }
        void AddDelayedFunctionCall(Uint8 delayMs, void* function, int param1 = 0, int param2 = 0, int param3 = 0);

        void InitializeWalls();
        void InitializeCharacters();
        void InitializeSlowAreas();

        void AddWall(Sint16 x, Sint16 y, Sint16 w = 50, Sint16 h = 50, bool breakable = false, bool visible = true);
        void AddWall(SDL_Rect2 wallRect) { wallRectangles.push_back(wallRect); }
        void AddMergedWall(Sint16 x, Sint16 y, Sint16 w = 50, Sint16 h = 50, bool breakable = false, bool visible = true);

        std::vector<SDL_Rect> GetSlowAreas() { return slowAreaRectangles; }
        bool IsInSlowArea(float x, float y);

    private:
        double pipeAngle;
        bool isRunning;
        SDL_Surface* screen;
        SDL_Event _event;
        std::vector<SDL_Rect2> wallRectangles;
        std::vector<SDL_Rect2> mergedWalls; //! To ease up collision detection.
        std::vector<SDL_Rect> slowAreaRectangles;
        std::vector<TemporarilySurfaces> temporarilySurfaces;
        std::vector<GrowingExplosions> growingExplosions;
        SDL_Rect mineExplosionRect[MAX_MINE_EXPLOSION_FRAMES];
        Player* player;
        std::vector<Bullet*> allBullets;
        std::vector<Landmine*> allLandmines;
        std::vector<Enemy*> enemies;
        int last_time, curr_time, diff_time;
};
