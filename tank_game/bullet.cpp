#include "game.h"

Bullet::Bullet(Game* _game, SDL_Surface* _screen, float x, float y, double _pipeAngle, bool _shooterIsPlr /* = true */, int _life /* = 2 */)
{
    if (!_game)
        return;

    game = _game;
    image = SDL_LoadBMP("bullet.bmp");
    xVelocity = PLAYER_BULLET_SPEED_X;
    yVelocity = PLAYER_BULLET_SPEED_Y;
    isRemoved = false;
    directionAngle = _pipeAngle;
    rotateAngle = _pipeAngle;
    screen = _screen;

    //! We maken de x en y co-ordinaten groter zodra de kogel gemaakt wordt. De X en Y as die gegeven zijn (hoofdletters) zijn het midden
    //! van de tank en niet de uitkomst van de pijp.
    posX = x + float(cos(directionAngle * M_PI / 180.0) * xVelocity) * 14.3f;
    posY = y - float(sin(directionAngle * M_PI / 180.0) * yVelocity) * 14.3f;

    bulletRect.x = Sint16(posX);
    bulletRect.y = Sint16(posY);
    bulletRect.w = BULLET_WIDTH;
    bulletRect.h = BULLET_HEIGHT;

    inSlowArea = false;

    //std::vector<SDL_Rect2> wallRects = game->GetWalls();
    //for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
    //{
    //    if ((*itr).visible && WillCollisionAt(&bulletRect, &(*itr)))
    //    {
    //        Explode(false);
    //        return;
    //    }
    //}

    SDL_SetColorKey(image, SDL_SRCCOLORKEY, COLOR_WHITE);
    rotatedBullet = rotozoomSurface(image, rotateAngle, 1.0, 0);
    SDL_BlitSurface(rotatedBullet, NULL, screen, &bulletRect);
    shooterIsPlr = _shooterIsPlr;
    life = _life;
}

Bullet::~Bullet()
{
    SDL_FreeSurface(image);
    isRemoved = true;
    delete this;
}

void Bullet::Explode(bool showExplosion /* = true */)
{
    if (showExplosion)
    {
        RGB explosionRGB;
        explosionRGB.r = 0x00;
        explosionRGB.g = 0x00;
        explosionRGB.b = 0x00;
        game->StoreSurfaceByTime("explosion.bmp", bulletRect, explosionRGB, 400);
    }

    //this->~Bullet();
    //! Just get rid of them...
    bulletRect.x = 5000;
    bulletRect.y = 5000;
    isRemoved = true;
    game->RemoveBullet(this);

    if (shooterIsPlr)
        if (Player* player = game->GetPlayer())
            player->DecrBulletCount();

    //game->UnregistrateBullet(this);
    //! TODO: FreeSurface
}

void Bullet::Update()
{
    if (isRemoved || !game || !game->IsRunning())
        return;

    SDL_Rect _bulletRect = { bulletRect.x, bulletRect.y, BULLET_WIDTH, BULLET_HEIGHT };
    _bulletRect.x -= rotatedBullet->w / 2 - image->w / 2;
    _bulletRect.y -= rotatedBullet->h / 2 - image->h / 2;

    SDL_SetColorKey(image, SDL_SRCCOLORKEY, COLOR_WHITE);
    rotatedBullet = rotozoomSurface(image, rotateAngle, 1.0, 0);
    SDL_BlitSurface(rotatedBullet, NULL, screen, &_bulletRect);

    //? TODO: directionAngle wordt alleen gegeven op constructor en is zelfde als pipeangle - wat was ik ook al weer denkende? o_o (hoe kan dit werken?)
    posX += float(cos(directionAngle * M_PI / 180.0) * xVelocity);
    posY -= float(sin(directionAngle * M_PI / 180.0) * yVelocity);
    _bulletRect.x = Sint16(posX);
    _bulletRect.y = Sint16(posY);

    if (game->IsInSlowArea(posX, posY))
    {
        if (!inSlowArea)
        {
            xVelocity /= 2;
            yVelocity /= 2;
            inSlowArea = true;
        }
    }
    else if (inSlowArea)
    {
        xVelocity *= 2;
        yVelocity *= 2;
        inSlowArea = false;
    }

    bool collision = false;
    bool showExplosion = true;

    if (life > 0)
    {
        if (Player* player = game->GetPlayer())
        {
            SDL_Rect plrRect;
            plrRect.x = Sint16(player->GetPosX());
            plrRect.y = Sint16(player->GetPosY());
            plrRect.w = PLAYER_WIDTH;
            plrRect.h = PLAYER_HEIGHT;
            collision = WillCollisionAt(&bulletRect, &plrRect);
        }

        std::vector<Bullet*> _bullets = game->GetAllBullets();
        SDL_Rect otherBulletRec;

        if (!collision)
        {
            if (!_bullets.empty())
            {
                for (std::vector<Bullet*>::iterator itr = _bullets.begin(); itr != _bullets.end(); ++itr)
                {
                    if ((*itr) != this && !(*itr)->isRemoved)
                    {
                        otherBulletRec = (*itr)->bulletRect;

                        if (WillCollisionAt(&bulletRect, &otherBulletRec))
                        {
                            (*itr)->SetRemainingLife(0); //! Allebei de kogels mogen kapot
                            collision = true;
                            break;
                        }
                    }
                }
            }

            if (!collision)
            {
                std::vector<Landmine*> _landmines = game->GetAllLandmines();

                if (!_landmines.empty())
                {
                    for (std::vector<Landmine*>::iterator itr = _landmines.begin(); itr != _landmines.end(); ++itr)
                    {
                        if (!(*itr)->IsRemoved() && WillCollisionAt(&bulletRect, &(*itr)->GetRectangle()))
                        {
                            showExplosion = false;
                            (*itr)->Explode();
                            life = 0;
                            return;
                        }
                    }
                }
            }

            std::vector<SDL_Rect2> wallRects = game->GetWalls();
            for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
            {
                if ((*itr).visible && WillCollisionAt(&_bulletRect, &(*itr)))
                {
                    bool hitLeftSide = false, hitRightSide = false, hitBottomSide = false, hitUpperSide = false, setWhichSideHit = false;

                    if (bulletRect.x + bulletRect.w - 2 <= (*itr).x) //! Left
                    {
                        hitLeftSide = true;
	                    setWhichSideHit = true;
                    }

                    if (!setWhichSideHit && (*itr).x + (*itr).w - 2 <= bulletRect.x) //! Right
                    {
                        hitRightSide = true;
	                    setWhichSideHit = true;
                    }

                    if (!setWhichSideHit && bulletRect.y + bulletRect.h -2 >= (*itr).y) //! Bottom
                    {
                        hitBottomSide = true;
	                    setWhichSideHit = false;
                    }

                    if (!setWhichSideHit && (*itr).y + (*itr).h - 2 >= bulletRect.y) //! Top
                    {
                        hitUpperSide = true;
	                    setWhichSideHit = false;
                    }

                    if (hitLeftSide || hitRightSide)
                    {
                        rotateAngle = 180 - rotateAngle;
                        xVelocity = -xVelocity;
                        posY += float(sin(directionAngle * M_PI / 180.0) * yVelocity) * 1.5f;
                        posX = hitLeftSide ? posX - 5 : posX + 5;
                    }
                    else if (hitBottomSide || hitUpperSide)
                    {
                        rotateAngle = -rotateAngle;
                        yVelocity = -yVelocity;
                        posX -= float(cos(directionAngle * M_PI / 180.0) * xVelocity) * 1.5f;
                        posY = hitUpperSide ? posY - 5 : posY + 5;
                    }

                    life--;
                    break;
                }
            }
        }

        //! TODO: Get rid of the 'shooterIsPlr' check. This is just temporarily so NPCs wont shoot themselves.
        if (shooterIsPlr && life > 0 && !collision)
        {
            std::vector<Enemy*> _enemies = game->GetEnemies();

            if (!_enemies.empty())
            {
                for (std::vector<Enemy*>::iterator itr = _enemies.begin(); itr != _enemies.end(); ++itr)
                {
                    if ((*itr)->IsAlive() && WillCollisionAt(&bulletRect, &(*itr)->GetRotatedBodyRect()))
                    {
                        (*itr)->JustDied();
                        Explode(false);
                        break;
                    }
                }
            }
        }
    }

    bulletRect.x = Sint16(posX);
    bulletRect.y = Sint16(posY);

    if (life <= 0 || collision)
        Explode(showExplosion);
}
