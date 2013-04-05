#include "game.h"

Bullet::Bullet(Game* _game, SDL_Surface* _screen, SDL_Surface* img, float X, float Y, int w, int h, int xVel, int yVel, int _life, double _pipeAngle)
{
    if (!_game)
        return;

    game = _game;
    image = img;
    xVelocity = xVel;
    yVelocity = yVel;
    life = _life;
    isRemoved = false;
    directionAngle = _pipeAngle;
    rotateAngle = _pipeAngle;
    screen = _screen;

    //! We maken de x en y co-ordinaten groter zodra de kogel gemaakt wordt. De X en Y as die gegeven zijn (hoofdletters) zijn het midden
    //! van de tank en niet de uitkomst van de pijp.
    x = X + float(cos(directionAngle * M_PI / 180.0) * xVelocity) * 14.3f;
    y = Y - float(sin(directionAngle * M_PI / 180.0) * yVelocity) * 14.3f;

    bulletRect.x = Sint16(x);
    bulletRect.y = Sint16(y);
    bulletRect.w = w;
    bulletRect.h = h;

    SDL_SetColorKey(image, SDL_SRCCOLORKEY, COLOR_WHITE);
    rotatedBullet = rotozoomSurface(image, rotateAngle, 1.0, 0);
    SDL_BlitSurface(rotatedBullet, NULL, screen, &bulletRect);
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

    //! TODO: directionAngle wordt alleen gegeven op constructor en is zelfde als pipeangle - wat was ik ook al weer denkende? o_o
    x += float(cos(directionAngle * M_PI / 180.0) * xVelocity);
    y -= float(sin(directionAngle * M_PI / 180.0) * yVelocity);
    _bulletRect.x = Sint16(x);
    _bulletRect.y = Sint16(y);

    if (x <= 0 || x + bulletRect.w >= screen->clip_rect.w)
    {
        rotateAngle = 180 - rotateAngle;
        xVelocity = -xVelocity;
        life--;
    }

    if (y <= 0 || y + bulletRect.h >= screen->clip_rect.h)
    {
        rotateAngle = -rotateAngle;
        yVelocity = -yVelocity;
        life--;
    }

    //if (WillCollisionAt(&bulletRect, &game->rotatingRectangle))
    //{
    //    //! Inkomend is in graden
    //    double inkomend = atan2(bulletRect.y - y, x - bulletRect.x) * M_PI / 180;
    //    directionAngle = -(45 - inkomend);
    //    rotateAngle = directionAngle * 180 / M_PI;
    //    //life--;
    //}

    bool collision = false;

    if (life > 0)
    {
        SDL_Rect plrRect;

        if (Player* player = game->GetPlayer())
        {
            plrRect.x = Sint16(player->GetPosX());
            plrRect.y = Sint16(player->GetPosY());
        }

        plrRect.w = PLAYER_WIDTH;
        plrRect.h = PLAYER_HEIGHT;

        collision = WillCollisionAt(&bulletRect, &plrRect);

        if (!collision)
        {
            std::vector<Bullet*> _bullets = game->GetAllBullets();
            SDL_Rect otherBulletRec;

            if (!_bullets.empty())
            {
                for (std::vector<Bullet*>::iterator itr = _bullets.begin(); itr != _bullets.end(); ++itr)
                {
                    if ((*itr) != this)
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
                        //if ((*itr) != this)
                        {
                            if (WillCollisionAt(&bulletRect, &(*itr)->GetRectangle()))
                            {
                                //! TODO: Alle dichtbijzijnde kogels, breakable walls en spelers targeten
                                //(*itr)->SetRemainingLife(0); //! Allebei de kogels mogen kapot
                                (*itr)->Explode();
                                Explode(false);
                                return; //! We roepen Bullet::Explode met een andere parameter als dan we zouden doen als life op 0 springt.
                            }
                        }
                    }
                }
            }
        }

        std::vector<SDL_Rect2> wallRects = game->GetWalls();
        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if (WillCollisionAt(&_bulletRect, &(*itr)))
            {
                bool hitLeftSide = false, hitRightSide = false, hitBottomSide = false, hitUpperSide = false;
                bool setWhichSideHit = false;

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

                //x -= float(cos(directionAngle * M_PI / 180.0) * xVelocity);
                //y += float(sin(directionAngle * M_PI / 180.0) * yVelocity);
                //x += float(cos(directionAngle * M_PI / 180.0) * xVelocity);
                //y -= float(sin(directionAngle * M_PI / 180.0) * yVelocity);

                if (hitLeftSide || hitRightSide)
                {
                    rotateAngle = 180 - rotateAngle;
                    xVelocity = -xVelocity;
                    y += float(sin(directionAngle * M_PI / 180.0) * yVelocity) * 1.5f;

                    if (hitLeftSide)
                        x -= 5;
                    else
                        x += 5;
                }
                else if (hitBottomSide || hitUpperSide)
                {
                    rotateAngle = -rotateAngle;
                    yVelocity = -yVelocity;
                    x -= float(cos(directionAngle * M_PI / 180.0) * xVelocity) * 1.5f;

                    if (hitUpperSide)
                        y -= 5;
                    else
                        y += 5;
                }

                life--;
                break;
            }
        }

        if (life > 0 && !collision)
        {
            std::vector<Enemy*> _enemies = game->GetEnemies();

            if (!_enemies.empty())
            {
                for (std::vector<Enemy*>::iterator itr = _enemies.begin(); itr != _enemies.end(); ++itr)
                {
                    if (WillCollisionAt(&bulletRect, &(*itr)->GetRotatedBodyRect()))
                    {
                        //XML!
                        collision = true;
                        break;
                    }
                }
            }
        }
    }

    bulletRect.x = Sint16(x);
    bulletRect.y = Sint16(y);

    if (life <= 0 || collision)
        Explode();
}
