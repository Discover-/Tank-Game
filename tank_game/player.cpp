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

    rectBody.x = Sint16(posX);
    rectBody.y = Sint16(posY);
    rectBody.w = PLAYER_WIDTH;
    rectBody.h = PLAYER_HEIGHT;
    rectPipe.x = Sint16(posX);
    rectPipe.y = Sint16(posY);
    rectPipe.w = PLAYER_WIDTH;
    rectPipe.h = PLAYER_HEIGHT;
}

void Player::Update()
{
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
        newX = Sint16(posX + float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]));
        plrRect.x = Sint16(newX);
        bool foundCollision = false;

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if ((*itr).visible && WillCollision(plrRect, (*itr)))
            {
                foundCollision = true;

                while (WillCollision(plrRect, (*itr)))
                {
                    newX -= 0.01f;
                    plrRect.x = Sint16(newX);
                }
            }
        }

        //! Pushing an enemy away logic.
        {
            if (!foundCollision)
            {
                std::vector<Enemy*> enemies = game->GetEnemies();

                for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
                {
                    if ((*itr)->IsAlive())
                    {
                        SDL_Rect currNpcRect = { Sint16((*itr)->GetPosX()), Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };

                        if (WillCollision(plrRect, currNpcRect))
                        {
                            foundCollision = true;

                            //? TODO: Take mass in consideration (use some alg. or not?)
                            float _newX = (*itr)->GetPosX() + float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
                            float _newY = (*itr)->GetPosY() - float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
                            SDL_Rect newNpcRect = { Sint16(_newX), Sint16(_newY), PLAYER_WIDTH, PLAYER_HEIGHT };

                            for (std::vector<SDL_Rect2>::iterator itrWall = wallRects.begin(); itrWall != wallRects.end(); ++itrWall)
                            {
                                if ((*itrWall).visible && WillCollision(newNpcRect, (*itrWall)))
                                {
                                    CollisionSide collisionSide = GetSideOfCollision(newNpcRect, (*itrWall).GetNormalRect());

                                    //! Move up or down ONLY.
                                    if (collisionSide == SIDE_RIGHT || collisionSide == SIDE_LEFT)
                                        _newX = (*itr)->GetPosX();
                                    //! Move to left or right ONLY.
                                    else if (collisionSide == SIDE_BOTTOM || collisionSide == SIDE_TOP)
                                        _newY = (*itr)->GetPosY();

                                    //! TODO: This will fix one issue but bring up another. If we are currently pushing an enemy into a corner and we only iterate over the first
                                    //! wall found in the iteration the enemy collides with and do this over and over, we will be pushed outside the game (through the walls).
                                    //! However, if we do NOT have this break here we will collide with more than one wall which often means we stop moving because both the _newX
                                    //! and the _newY variables are set to not be changed as we, for example, hit the upper side of one wall but the left side of another.
                                    break;
                                }
                            }

                            (*itr)->SetPosX(_newX);
                            (*itr)->SetPosY(_newY);
                        }
                    }
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
            if ((*itr).visible && WillCollision(plrRect, (*itr)))
            {
                foundCollision = true;

                while (WillCollision(plrRect, (*itr)))
                {
                    newY += 0.01f;
                    plrRect.y = Sint16(newY);
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
        newX = Sint16(posX - float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]));
        plrRect.x = Sint16(newX);
        bool foundCollision = false;

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if ((*itr).visible && WillCollision(plrRect, (*itr)))
            {
                foundCollision = true;

                while (WillCollision(plrRect, (*itr)))
                {
                    newX += 0.01f;
                    plrRect.x = Sint16(newX);
                }
            }
        }

        //! Pushing an enemy away logic.
        {
            if (!foundCollision)
            {
                std::vector<Enemy*> enemies = game->GetEnemies();

                for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
                {
                    if ((*itr)->IsAlive())
                    {
                        SDL_Rect currNpcRect = { Sint16((*itr)->GetPosX()), Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };

                        if (WillCollision(plrRect, currNpcRect))
                        {
                            foundCollision = true;

                            //? TODO: Take mass in consideration (use some alg. or not?)
                            float _newX = (*itr)->GetPosX() - float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
                            float _newY = (*itr)->GetPosY() + float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
                            SDL_Rect newNpcRect = { Sint16(_newX), Sint16((*itr)->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };

                            for (std::vector<SDL_Rect2>::iterator itrWall = wallRects.begin(); itrWall != wallRects.end(); ++itrWall)
                            {
                                if ((*itrWall).visible && WillCollision(newNpcRect, (*itrWall)))
                                {
                                    CollisionSide collisionSide = GetSideOfCollision(newNpcRect, (*itrWall).GetNormalRect());

                                    //! Move up or down ONLY.
                                    if (collisionSide == SIDE_RIGHT || collisionSide == SIDE_LEFT)
                                        _newX = (*itr)->GetPosX();
                                    //! Move to left or right ONLY.
                                    else if (collisionSide == SIDE_BOTTOM || collisionSide == SIDE_TOP)
                                        _newY = (*itr)->GetPosY();

                                    //! TODO: This will fix one issue but bring up another. If we are currently pushing an enemy into a corner and we only iterate over the first
                                    //! wall found in the iteration the enemy collides with and do this over and over, we will be pushed outside the game (through the walls).
                                    //! However, if we do NOT have this break here we will collide with more than one wall which often means we stop moving because both the _newX
                                    //! and the _newY variables are set to not be changed as we, for example, hit the upper side of one wall but the left side of another.
                                    break;
                                }
                            }

                            (*itr)->SetPosX(_newX);
                            (*itr)->SetPosY(_newY);
                        }
                    }
                }
            }
        }

        if (!foundCollision)
            posX -= float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]);

        foundCollision = false;
        newY = (posY + float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_BACKWARD]));
        plrRect.y = Sint16(newY);

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if ((*itr).visible && WillCollision(plrRect, (*itr)))
            {
                foundCollision = true;

                while (WillCollision(plrRect, (*itr)))
                {
                    newY -= 0.01f;
                    plrRect.y = Sint16(newY);
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

    rectBody.x = Sint16(posX);
    rectBody.y = Sint16(posY);
    rectPipe.x = Sint16(posX);
    rectPipe.y = Sint16(posY);
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

void Player::SetKeysDown(Uint8 index, bool value)
{
    keysDown[index] = value;

    if (value)
    {
        switch (index)
        {
            case 0: //! ^ & W
                keysDown[2] = false;
                break;
            case 2: //! v & A
                keysDown[0] = false;
                break;
            case 1: //! < & A
                keysDown[3] = false;
                break;
            case 3: //! > & D
                keysDown[1] = false;
                break;
            default:
                break;
        }
    }
}
