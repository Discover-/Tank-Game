#include "game.h"

Landmine::Landmine(Game* _game, SDL_Surface* _screen, float x, float y)
{
    if (!_game)
        return;

    game = _game;
    screen = _screen;
    imageNormal = SDL_LoadBMP("landmine_initial.bmp");
    imageTimer = SDL_LoadBMP("landmine_timer.bmp");
    posX = x;
    posY = y;
    landmineRect.x = Sint16(x);
    landmineRect.y = Sint16(y);
    landmineRect.w = LANDMINE_WIDTH;
    landmineRect.h = LANDMINE_HEIGHT;
    isRemoved = false;
    explosionDelay = 3000;
    timerTillExplode = 15000;
    countdownTenMs = 0;
    timerImage = false;
    steppedOn = false;

    SDL_SetColorKey(imageNormal, SDL_SRCCOLORKEY, COLOR_WHITE);
    SDL_BlitSurface(imageNormal, NULL, screen, &landmineRect);
}

Landmine::~Landmine()
{
    SDL_FreeSurface(imageNormal);
    isRemoved = true;
    delete this;
}

void Landmine::Explode(bool showExplosion /* = true */)
{
    if (showExplosion)
        game->AddBigExplosion(float(landmineRect.x - 5), float(landmineRect.y - 5), 0, 80);

    isRemoved = true;

    //RGB explosionRGB;
    //explosionRGB.r = 0x00;
    //explosionRGB.g = 0x00;
    //explosionRGB.b = 0x00;
    //game->StoreSurfaceByTime("explosion_big.bmp", landmineRect, explosionRGB, 400);

    SDL_Rect centeredRect = landmineRect;
    centeredRect.x += centeredRect.w / 2;
    centeredRect.y += centeredRect.h / 2;

    std::vector<SDL_Rect2>& wallRects = game->GetWalls();
    for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); )
    {
        if ((*itr).breakable && (*itr).visible)
        {
            if (IsInRange(centeredRect.x, (*itr).x, centeredRect.y, (*itr).y, 100.0f))
            {
                (*itr).visible = false;
                itr = wallRects.begin();
            }
            else
                ++itr;
        }
        else
            ++itr;
    }

    std::vector<SDL_Rect2>& mergedWallRects = game->GetMergedWalls();
    for (std::vector<SDL_Rect2>::iterator itr = mergedWallRects.begin(); itr != mergedWallRects.end(); )
    {
        if ((*itr).breakable && (*itr).visible)
        {
            if (IsInRange(centeredRect.x, (*itr).x, centeredRect.y, (*itr).y, 100.0f))
            {
                (*itr).visible = false;
                itr = mergedWallRects.begin();
            }
            else
                ++itr;
        }
        else
            ++itr;
    }

    std::vector<Bullet*> _bullets = game->GetAllBullets();

    if (!_bullets.empty())
        for (std::vector<Bullet*>::iterator itr = _bullets.begin(); itr != _bullets.end(); ++itr)
            if (!(*itr)->IsRemoved() && IsInRange(centeredRect.x, (*itr)->GetPosX(), centeredRect.y, (*itr)->GetPosY(), 100.0f))
                (*itr)->Explode();

    std::vector<Landmine*> _landmines = game->GetAllLandmines();

    if (!_landmines.empty())
        for (std::vector<Landmine*>::iterator itr = _landmines.begin(); itr != _landmines.end(); ++itr)
            if ((*itr) != this && !(*itr)->IsRemoved() && IsInRange(centeredRect.x, (*itr)->GetPosX() + landmineRect.w / 2, centeredRect.y, (*itr)->GetPosY() + landmineRect.w / 2, 100.0f))
                (*itr)->Explode();

    std::vector<Enemy*> _enemies = game->GetEnemies();

    if (!_enemies.empty())
        for (std::vector<Enemy*>::iterator itr = _enemies.begin(); itr != _enemies.end(); ++itr)
            if ((*itr)->IsAlive() && IsInRange(centeredRect.x, (*itr)->GetPosX(), centeredRect.y, (*itr)->GetPosY(), 60.0f))
                (*itr)->JustDied();

    //if (Player* player = game->GetPlayer())
    //    if ((*itr)->IsAlive() && IsInRange(landmineRect.x, (*itr)->GetPosX(), landmineRect.y, (*itr)->GetPosY(), 60.0f))
    //        (*itr)->JustDied();

    /*SDL_Surface* tmpExplosion = SDL_LoadBMP("mine_explosion.bmp");
    SDL_Surface* spriteExplosion = SDL_DisplayFormat(tmpExplosion);
    SDL_FreeSurface(tmpExplosion);
    SDL_SetColorKey(spriteExplosion, SDL_SRCCOLORKEY, COLOR_WHITE);*/

    //! Just get rid of them...
    landmineRect.x = 5000;
    landmineRect.y = 5000;
    game->RemoveLandmine(this);

    if (Player* player = game->GetPlayer())
        player->DecrLandmineCount();

    //game->UnregistrateLandmine(this);
    //! TODO: FreeSurface
}

void Landmine::Update()
{
    if (!game || !game->IsRunning() || isRemoved)
        return;

    SDL_SetColorKey(timerImage ? imageTimer : imageNormal, SDL_SRCCOLORKEY, COLOR_WHITE);
    SDL_BlitSurface(timerImage ? imageTimer : imageNormal, NULL, screen, &landmineRect);

    if (!steppedOn)
    {
        if (!explosionDelay)
        {
            if (Player* player = game->GetPlayer())
            {
                SDL_Rect plrRect;
                plrRect.x = Sint16(player->GetPosX());
                plrRect.y = Sint16(player->GetPosY());
                plrRect.w = PLAYER_WIDTH;
                plrRect.h = PLAYER_HEIGHT;

                if (WillCollision(landmineRect, plrRect))
                {
                    timerTillExplode = 50;
                    steppedOn = true;
                    return;
                }
            }
        }

        std::vector<Enemy*> enemies = game->GetEnemies();

        for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
        {
            if ((*itr)->IsAlive())
            {
                SDL_Rect npcRect;
                npcRect.x = Sint16((*itr)->GetPosX());
                npcRect.y = Sint16((*itr)->GetPosY());
                npcRect.w = 45;
                npcRect.h = 51;

                if (WillCollision(landmineRect, npcRect))
                {
                    timerTillExplode = 50;
                    steppedOn = true;
                    return;
                }
            }
        }
    }
}

void Landmine::HandleTimers(unsigned int diff_time)
{
    if (explosionDelay)
    {
        if (diff_time >= explosionDelay)
            explosionDelay = 0;
        else
            explosionDelay -= diff_time;
    }

    if (timerTillExplode)
    {
        if (diff_time >= timerTillExplode)
        {
            countdownTenMs += 10;
            timerTillExplode = 300 - countdownTenMs;
            timerImage = !timerImage;
        }
        else
            timerTillExplode -= diff_time;
    }
    else if (countdownTenMs)
    {
        countdownTenMs = 0;
        Explode();
    }
}
