#include "game.h"

Player::Player(Game* _game, float x, float y)
{
    posX = x;
    posY = y;
    game = _game;
    //pipeAngle = 0;
    canShoot = true;
    shootCooldown = 0;
    canPlaceLandmine = true;
    landmineCooldown = 0;
    bulletCount = 0;

    for (int i = 0; i < 4; ++i)
        keysDown[i] = false;

    if (game)
        screen = game->GetScreen();
}

void Player::Update()
{
    //! We moeten deze event iedere updatecall opnieuw oproepen.
    //SDL_Event _event = game->GetEvent();

    float newX = 0.0f;
    float newY = 0.0f;
    //! otherOutcomeX/Y zijn variabelen die houden wat de nieuwe destination zou zijn zonder collision checks.
    Sint16 otherOutcomeX = 0;
    Sint16 otherOutcomeY = 0;
    SDL_Rect plrRect = { Sint16(posX), Sint16(posY), 51, 45 };
    std::vector<SDL_Rect2> wallRects = game->GetWalls();

    if (keysDown[0])
    {
        //posX += float(cos(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_FORWARD);
        //posY -= float(sin(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_FORWARD);

        //std::vector<SDL_Rect2> wallRects = game->GetWalls();
        //for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        //{
        //    if (WillCollisionAt(&plrRect, &(*itr)))
        //    {
        //        posX -= float(cos(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_FORWARD);
        //        posY += float(sin(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_FORWARD);
        //        break;
        //    }
        //}

        newX = Sint16(posX + float(cos(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_FORWARD));
        otherOutcomeX = Sint16(newX);
        plrRect.x = Sint16(newX);
        bool foundCollision = false;

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if ((*itr).visible && WillCollisionAt(&plrRect, &(*itr)))
            {
                foundCollision = true;

                while (true)
                {
                    newX -= 0.01f;
                    plrRect.x = Sint16(newX);

                    if (!WillCollisionAt(&plrRect, &(*itr)) || newX >= otherOutcomeX)
                        break;
                }

                break;
            }
        }

        if (!foundCollision)
        {
            std::vector<Enemy*> enemies = game->GetEnemies();
            bool foundCollisionWithNpc = false;
            bool foundCollisionNpcNewPos = false;

            for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
            {
                SDL_Rect currNpcRect = { Sint16((*itr)->GetPosX()), Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };
                float _newX = (*itr)->GetPosX() - 2;

                if (WillCollisionAt(&plrRect, &currNpcRect))
                {
                    foundCollision = true;
                    foundCollisionWithNpc = true;
                    Sint16 _otherOutcomeX = Sint16(_newX);
                    SDL_Rect newNpcRect = { _newX, Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };

                    for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
                    {
                        if ((*itr).visible && WillCollisionAt(&newNpcRect, &(*itr)))
                        {
                            foundCollisionNpcNewPos = true;

                            while (true)
                            {
                                _newX += 0.01f;
                                newNpcRect.x = Sint16(_newX);

                                if (!WillCollisionAt(&newNpcRect, &(*itr)) || _newX >= _otherOutcomeX)
                                    break;
                            }
                        }
                    }

                    //break;
                }

                if (foundCollisionWithNpc)
                {
                    if (foundCollisionNpcNewPos)
                        (*itr)->SetPosX(Sint16(_newX));
                    else
                        (*itr)->SetPosX((*itr)->GetPosX() - 2);
                }
            }
        }

        if (!foundCollision)
            posX += float(cos(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_FORWARD);

        foundCollision = false;
        newY = (posY - float(sin(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_FORWARD));
        otherOutcomeY = Sint16(newY);
        plrRect.y = Sint16(newY);

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if (WillCollisionAt(&plrRect, &(*itr)))
            {
                foundCollision = true;

                while (true)
                {
                    newY += 0.01f;
                    plrRect.y = Sint16(newY);

                    if (!WillCollisionAt(&plrRect, &(*itr)) || newY >= otherOutcomeY)
                        break;
                }

                break;
            }
        }

        if (!foundCollision)
            posY -= float(sin(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_FORWARD);
    }

    newX = 0.0f;
    newY = 0.0f;
    otherOutcomeX = 0;
    otherOutcomeY = 0;
    plrRect.x = Sint16(posX);
    plrRect.y = Sint16(posY);

    if (keysDown[2])
    {
        //posX -= float(cos(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_BACKWARD);
        //posY += float(sin(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_BACKWARD);

        //std::vector<SDL_Rect2> wallRects = game->GetWalls();
        //for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        //{
        //    if (WillCollisionAt(&plrRect, &(*itr)))
        //    {
        //        posX += float(cos(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_BACKWARD);
        //        posY -= float(sin(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_BACKWARD);
        //        break;
        //    }
        //}

        newX = Sint16(posX - float(cos(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_BACKWARD));
        otherOutcomeX = Sint16(newX);
        plrRect.x = otherOutcomeX;
        bool foundCollision = false;

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if ((*itr).visible && WillCollisionAt(&plrRect, &(*itr)))
            {
                foundCollision = true;

                while (true)
                {
                    newX += 0.01f;
                    plrRect.x = Sint16(newX);

                    if (!WillCollisionAt(&plrRect, &(*itr)) || newX >= otherOutcomeX)
                        break;
                }

                break;
            }
        }

        if (!foundCollision)
            posX -= float(cos(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_BACKWARD);

        foundCollision = false;

        newY = (posY + float(sin(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_BACKWARD));
        otherOutcomeY = Sint16(newY);
        plrRect.y = otherOutcomeY;

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if ((*itr).visible && WillCollisionAt(&plrRect, &(*itr)))
            {
                foundCollision = true;

                while (true)
                {
                    newY -= 0.01f;
                    plrRect.y = Sint16(newY);

                    if (!WillCollisionAt(&plrRect, &(*itr)) || newY >= otherOutcomeY)
                        break;
                }

                break;
            }
        }

        if (!foundCollision)
            posY += float(sin(movingAngle * M_PI / 180.0) * PLAYER_MOVES_SPEED_BACKWARD);
    }

    if (keysDown[1])
        movingAngle += 3;

    if (keysDown[3])
        movingAngle -= 3;

    if (posX < 0)
        posX = 0;

    if (posY < 0)
        posY = 0;

    if (posX > 938)
        posX = 938;

    if (posY > 544)
        posY = 544;
}

void Player::HandleTimers(unsigned int diff_time)
{
    if (!canShoot)
    {
        if (diff_time >= shootCooldown)
        {
            shootCooldown = 0;
            canShoot = true;
        }
        else
            shootCooldown -= diff_time;
    }

    if (!canPlaceLandmine)
    {
        if (diff_time >= landmineCooldown)
        {
            landmineCooldown = 0;
            canPlaceLandmine = true;
        }
        else
            landmineCooldown -= diff_time;
    }
}
