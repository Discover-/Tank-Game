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
    moveSpeed[MOVE_TYPE_FORWARD] = PLAYER_MOVES_SPEED_FORWARD;
    moveSpeed[MOVE_TYPE_BACKWARD] = PLAYER_MOVES_SPEED_BACKWARD;
    inSlowArea = false;

    for (int i = 0; i < 4; ++i)
        keysDown[i] = false;

    if (game)
        screen = game->GetScreen();
}

void Player::Update()
{
    //! We moeten deze event iedere updatecall opnieuw oproepen.
    //SDL_Event _event = game->GetEvent();

    if (game->IsInSlowArea(posX, posY))
    {
        if (!inSlowArea)
        {
            for (int i = 0; i < MOVE_TYPE_MAX; ++i)
                moveSpeed[i] /= 2;

            inSlowArea = true;
        }
    }
    else if (inSlowArea)
    {
        for (int i = 0; i < MOVE_TYPE_MAX; ++i)
            moveSpeed[i] *= 2;

        inSlowArea = false;
    }

    float newX = 0.0f;
    float newY = 0.0f;
    SDL_Rect plrRect = { Sint16(posX), Sint16(posY), 51, 45 };
    std::vector<SDL_Rect2> wallRects = game->GetWalls();

    if (keysDown[0])
    {
        //posX += float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
        //posY -= float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);

        //std::vector<SDL_Rect2> wallRects = game->GetWalls();
        //for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        //{
        //    if (WillCollisionAt(&plrRect, &(*itr)))
        //    {
        //        posX -= float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
        //        posY += float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
        //        break;
        //    }
        //}
        //posX = 400
        //newX = 500
        //othX = 500
        newX = Sint16(posX + float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]));
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

                    if (!WillCollisionAt(&plrRect, &(*itr)))
                        break;
                }
            }
        }

        //! Pushing an enemy away logic.
        if (!foundCollision)
        {
            std::vector<Enemy*> enemies = game->GetEnemies();
            bool foundCollisionWithNpc = false;
            bool foundCollisionNpcNewPos = false;

            for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
            {
                if ((*itr)->IsAlive())
                {
                    SDL_Rect currNpcRect = { Sint16((*itr)->GetPosX()), Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };
                    float _newX = (*itr)->GetPosX() - 2;

                    if (WillCollisionAt(&plrRect, &currNpcRect))
                    {
                        foundCollision = true;
                        foundCollisionWithNpc = true;
                        SDL_Rect newNpcRect = { Sint16(_newX), Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };

                        for (std::vector<SDL_Rect2>::iterator itrWall = wallRects.begin(); itrWall != wallRects.end(); ++itrWall)
                        {
                            if ((*itrWall).visible && WillCollisionAt(&newNpcRect, &(*itrWall)))
                            {
                                foundCollisionNpcNewPos = true;

                                while (true)
                                {
                                    _newX += 0.01f;
                                    newNpcRect.x = Sint16(_newX);

                                    if (!WillCollisionAt(&newNpcRect, &(*itrWall)))
                                        break;
                                }
                            }
                        }
                    }

                    if (foundCollisionWithNpc)
                    {
                        if (foundCollisionNpcNewPos)
                            (*itr)->SetPosX(Sint16(_newX));
                        else
                            (*itr)->SetPosX((*itr)->GetPosX() - 2);
                    }

                    //! TEMP: This will prevent the player from pushing more than one enemy at the same time. Just a temporarily testing thing.
                    if (foundCollision)
                        break;
                }
            }
        }

        if (!foundCollision)
            posX += float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);

        foundCollision = false;
        newY = (posY - float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]));
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

                    if (!WillCollisionAt(&plrRect, &(*itr)))
                        break;
                }
            }
        }

        if (!foundCollision)
        {
            std::vector<Enemy*> enemies = game->GetEnemies();
            bool foundCollisionWithNpc = false;
            bool foundCollisionNpcNewPos = false;

            for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
            {
                if ((*itr)->IsAlive())
                {
                    SDL_Rect currNpcRect = { Sint16((*itr)->GetPosX()), Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };
                    float _newY = (*itr)->GetPosY() - 2;

                    if (WillCollisionAt(&plrRect, &currNpcRect))
                    {
                        foundCollision = true;
                        foundCollisionWithNpc = true;
                        SDL_Rect newNpcRect = { Sint16(_newY), Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };

                        for (std::vector<SDL_Rect2>::iterator itrWall = wallRects.begin(); itrWall != wallRects.end(); ++itrWall)
                        {
                            if ((*itrWall).visible && WillCollisionAt(&newNpcRect, &(*itrWall)))
                            {
                                foundCollisionNpcNewPos = true;

                                while (true)
                                {
                                    _newY += 0.01f;
                                    newNpcRect.y = Sint16(_newY);

                                    if (!WillCollisionAt(&newNpcRect, &(*itrWall)))
                                        break;
                                }
                            }
                        }
                    }

                    if (foundCollisionWithNpc)
                    {
                        if (foundCollisionNpcNewPos)
                            (*itr)->SetPosY(Sint16(_newY));
                        else
                            (*itr)->SetPosY((*itr)->GetPosY() - 2);
                    }

                    //! TEMP: This will prevent the player from pushing more than one enemy at the same time. Just a temporarily testing thing.
                    if (foundCollision)
                        break;
                }
            }
        }

        if (!foundCollision)
            posY -= float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
    }

    newX = 0.0f;
    newY = 0.0f;
    plrRect.x = Sint16(posX);
    plrRect.y = Sint16(posY);

    if (keysDown[2])
    {
        //posX -= float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]);
        //posY += float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]);

        //std::vector<SDL_Rect2> wallRects = game->GetWalls();
        //for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        //{
        //    if (WillCollisionAt(&plrRect, &(*itr)))
        //    {
        //        posX += float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]);
        //        posY -= float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]);
        //        break;
        //    }
        //}

        newX = Sint16(posX - float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]));
        plrRect.x = newX;
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

                    if (!WillCollisionAt(&plrRect, &(*itr)))
                        break;
                }
            }
        }

        if (!foundCollision)
            posX -= float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]);

        foundCollision = false;

        newY = (posY + float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]));
        plrRect.y = newY;

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if ((*itr).visible && WillCollisionAt(&plrRect, &(*itr)))
            {
                foundCollision = true;

                while (true)
                {
                    newY -= 0.01f;
                    plrRect.y = Sint16(newY);

                    if (!WillCollisionAt(&plrRect, &(*itr)))
                        break;
                }
            }
        }

        if (!foundCollision)
            posY += float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]);
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
