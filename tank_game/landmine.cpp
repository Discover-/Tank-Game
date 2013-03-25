#include "game.h"

Landmine::Landmine(Game* _game, SDL_Surface* _screen, SDL_Surface* img, float x, float y, int w, int h)
{
    if (!_game)
        return;

    game = _game;
    image = img;
    landmineRect.x = Sint16(x);
    landmineRect.y = Sint16(y);
    landmineRect.w = w;
    landmineRect.h = h;
    isRemoved = false;
    screen = _screen;

    SDL_SetColorKey(image, SDL_SRCCOLORKEY, COLOR_WHITE);
    SDL_BlitSurface(image, NULL, screen, &landmineRect);
}

Landmine::~Landmine()
{
    SDL_FreeSurface(image);
    isRemoved = true;
    delete this;
}

void Landmine::Explode()
{
    MineExplosions mineExplosion;
    mineExplosion.x = Sint16(landmineRect.x);// + (landmineRect.w / 2));
    mineExplosion.y = Sint16(landmineRect.y);// + (landmineRect.h / 2));
    mineExplosion.frame = 0;
    mineExplosion.delay = 80;
    game->AddLandMineExplosion(mineExplosion);

    //RGB explosionRGB;
    //explosionRGB.r = 0x00;
    //explosionRGB.g = 0x00;
    //explosionRGB.b = 0x00;
    //game->StoreSurfaceByTime("explosion_big.bmp", landmineRect, explosionRGB, 400);
    
    //! TODO: reference zodat er geen Game::SetWalls nodig is.
    std::vector<SDL_Rect2> wallRects = game->GetWalls();
    for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); )
    {
        if ((*itr).breakable)
        {
            float dx = float((*itr).x - landmineRect.x);
            float dy = float((*itr).y - landmineRect.y);
            //float dist = sqrt(double((dx * dx) + (dy * dy)));
            //sqrt( pow( x2 - x1, 2 ) + pow( y2 - y1, 2 ) );

            //if (dist > 0 && dist <= 20)
            {
                wallRects.erase(itr++);
                itr = wallRects.begin();
            }
            //else
            //    ++itr;
        }
        else
            ++itr;
    }

    game->SetWalls(wallRects);

    /*SDL_Surface* tmpExplosion = SDL_LoadBMP("mine_explosion.bmp");
    SDL_Surface* spriteExplosion = SDL_DisplayFormat(tmpExplosion);
    SDL_FreeSurface(tmpExplosion);
    SDL_SetColorKey(spriteExplosion, SDL_SRCCOLORKEY, COLOR_WHITE);*/

    /*Animation animatedExplosion;
    animatedExplosion.MaxFrames = 12;
    animatedExplosion.Oscillate = false;
    animatedExplosion.OnAnimate();

    //CSurface::OnDraw(Surf_Display, Surf_Test, 290, 220, 0, Anim_Yoshi.GetCurrentFrame() * 64, 64, 64);
	SDL_BlitSurface(tmpExplosion, NULL, game->GetScreen(), &box);*/

    //this->~Landmine();
    //! Just get rid of them...
    landmineRect.x = 5000;
    landmineRect.y = 5000;
    isRemoved = true;

    if (Player* player = game->GetPlayer())
        player->DecrLandmineCount();

    //game->UnregistrateLandmine(this);
    //! TODO: FreeSurface
}

void Landmine::Update()
{
    if (!game || !game->IsRunning() || isRemoved)
        return;

    SDL_SetColorKey(image, SDL_SRCCOLORKEY, COLOR_WHITE);
    SDL_BlitSurface(image, NULL, screen, &landmineRect);

    SDL_Rect plrRect;

    if (Player* player = game->GetPlayer())
    {
        plrRect.x = Sint16(player->GetPosX());
        plrRect.y = Sint16(player->GetPosY());
        plrRect.w = PLAYER_WIDTH;
        plrRect.h = PLAYER_HEIGHT;
        //plrRect.w = 45;
        //plrRect.h = 50;

        if (WillCollisionAt(&landmineRect, &plrRect))
        {
            Explode();
            return;
        }
    }

    std::vector<Enemy*> enemies = game->GetEnemies();
    for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
    {
        SDL_Rect npcRect;
        npcRect.x = Sint16((*itr)->GetPosX());
        npcRect.y = Sint16((*itr)->GetPosY());
        npcRect.w = 45;
        npcRect.h = 51;

        if (WillCollisionAt(&landmineRect, &npcRect))
        {
            Explode();
            return;
        }
    }
}
