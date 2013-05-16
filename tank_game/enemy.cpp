#include "game.h"

Enemy::Enemy(Game* _game, float x, float y, SDL_Surface* body, SDL_Surface* pipe, SDL_Rect _rectBody, SDL_Rect _rectPipe)
{
    if (!_game)
        return;

    posX = x;
    posY = y;
    startPosX = x;
    startPosY = y;
    game = _game;
    //pipeAngle = 0;
    canShoot = true;
    shootCooldown = 0;
    canPlaceLandmine = true;
    landmineCooldown = 0;
    bodySprite = body;
    pipeSprite = pipe;
    rectBody = _rectBody;
    rectPipe = _rectPipe;
    rotatedBody = NULL;
    rotatedPipe = NULL;
    rotatingBodyAngle = 1.0f;
    rotatingPipeAngle = 1.0f;
    screen = game->GetScreen();
    lastPointIncreaseTime = 0;
    isDead = false;
    randomShootTimer = urand(4000, 10000);
    moveSpeed[MOVE_TYPE_FORWARD] = NPC_MOVES_SPEED_FORWARD;
    moveSpeed[MOVE_TYPE_BACKWARD] = NPC_MOVES_SPEED_BACKWARD;
    inSlowArea = false;
    bulletCount = 0;
}

void Enemy::Update()
{
    if (isDead)
        return;

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

    Player* player = game->GetPlayer();

    if (!player)
        return;

    if (canShoot && !randomShootTimer)// && bulletCount < PLAYER_MAX_BULLETS)
    {
        float bulletX = float(posX + (PLAYER_WIDTH / 2) - 12) + (16 / 2);
        float bulletY = float(posY + (PLAYER_HEIGHT / 2) - 12) + (16 / 2);
        float actualX = bulletX + float(cos(rotatingPipeAngle * M_PI / 180.0) * PLAYER_BULLET_SPEED_X) * 14.3f;
        float actualY = bulletY + float(sin(rotatingPipeAngle * M_PI / 180.0) * PLAYER_BULLET_SPEED_Y) * 14.3f;
        float bulletNewX = bulletX;
        float bulletNewY = bulletY;
        float bulletVelX = PLAYER_BULLET_SPEED_X;
        float bulletVelY = PLAYER_BULLET_SPEED_Y;
        int hitWallTimes = 0;
        bool hitsWallOnStart = false;
        bool hitsEnemyOnPath = false;
        SDL_Rect bulletRect = { Sint16(actualX), Sint16(actualY), BULLET_WIDTH, BULLET_HEIGHT };
        std::vector<SDL_Rect2> wallRects = game->GetWalls();

        if (!IsInRange(posX, player->GetPosX(), posY, player->GetPosY(), 80.0f))
        {
            for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
            {
                if ((*itr).visible && WillCollision(&bulletRect, &(*itr)))
                {
                    hitsWallOnStart = true;
                    break;
                }
            }

            if (!hitsWallOnStart)
            {
                for (int i = 0; i < 300; ++i)
                {
                    bulletNewX += float(cos(rotatingPipeAngle * M_PI / 180.0) * bulletVelX);
                    bulletNewY -= float(sin(rotatingPipeAngle * M_PI / 180.0) * bulletVelY);
                    bulletRect.x = Sint16(bulletNewX);
                    bulletRect.y = Sint16(bulletNewY);
                    //hitsEnemyOnPath = false;

                    for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
                    {
                        if ((*itr).visible && WillCollision(&bulletRect, &(*itr)) && hitWallTimes < PLAYER_BULLET_LIFES)
                        {
                            CollisionSide collisionSide = GetSideOfCollision(&bulletRect, &(*itr).GetNormalRect());

                            if (collisionSide == SIDE_LEFT || collisionSide == SIDE_RIGHT)
                            {
                                hitWallTimes++;
                                bulletVelX = -bulletVelX;
                                bulletNewY += float(sin(rotatingPipeAngle * M_PI / 180.0) * bulletVelX) * 1.5f;
                                bulletNewX = collisionSide == SIDE_LEFT ? bulletNewX - 5 : bulletNewX + 5;
                            }
                            else if (collisionSide == SIDE_BOTTOM || collisionSide == SIDE_TOP)
                            {
                                hitWallTimes++;
                                bulletVelY = -bulletVelY;
                                bulletNewX -= float(cos(rotatingPipeAngle * M_PI / 180.0) * bulletVelY) * 1.5f;
                                bulletNewY = collisionSide == SIDE_TOP ? bulletNewY - 5 : bulletNewY + 5;
                            }

                            break;
                        }
                    }

                    std::vector<Enemy*> _enemies = game->GetEnemies();

                    if (!_enemies.empty())
                    {
                        for (std::vector<Enemy*>::iterator itr = _enemies.begin(); itr != _enemies.end(); ++itr)
                        {
                            if ((*itr)->IsAlive() && WillCollision(&(*itr)->GetRotatedBodyRect(), &bulletRect))
                            {
                                //hitsEnemyOnPath = true;
                                break;
                            }
                        }
                    }

                    if (hitsEnemyOnPath)
                        break;

                    if (WillCollision(&player->GetRectBody(), &bulletRect))
                    {
                        if (Bullet* bullet = new Bullet(game, screen, bulletX, bulletY, rotatingPipeAngle, false))
                        {
                            game->AddBullet(bullet);
                            //AddBullet(bullet);
                            canShoot = false;
                            shootCooldown = 200;
                            bulletCount++;
                            break;
                        }
                    }
                }

                randomShootTimer = urand(100, 400);
            }
        }
    }

    if (!waypoints.empty())
    {
        bool turningToNextPoint = false;
        float xDestTemp = 0.0f;
        float yDestTemp = 0.0f;

        for (std::vector<WaypointInformation>::iterator itr = waypoints.begin(); itr != waypoints.end(); )
        {
            Uint32 lastPointId = 0;
            Uint32 pointId = itr->currDestPointId;
            float xDest = 0.0f;
            float yDest = 0.0f;
            bool incrItr = true;

            for (std::vector<WaypointNode>::iterator itr2 = itr->nodes.begin(); itr2 != itr->nodes.end(); ++itr2)
            {
                Uint32 nextPointId = itr2->pointId;

                if (pointId == nextPointId)
                {
                    xDest = itr2->x;
                    yDest = itr2->y;
                }

                lastPointId = nextPointId;
            }

            //if (!(xDest == 0.0f || yDest == 0.0f || (xDest == posX && yDest == posY)))
            if (!(xDest == 0.0f || yDest == 0.0f))
            {
                //if (turningToNextPoint)
                {
                    if (xDestTemp == 0.0f && yDestTemp == 0.0f)
                    {
                        xDestTemp = xDest;
                        yDestTemp = yDest;
                    }
                }
                //else if (xDestTemp != 0.0f || yDestTemp != 0.0f)
                //{
                //    xDestTemp = 0.0f;
                //    yDestTemp = 0.0f;
                //}

                //if (!turningToNextPoint)
                    movingAngle = float(atan2(posY - yDest, xDest - posX) * 180 / M_PI);
                    //rotatingBodyAngle = movingAngle;

                //if ((posX < xDestTemp - 50 && posX > xDestTemp + 50 && posY < yDestTemp - 50 && posY > yDestTemp + 50) ||
                //    (posX > xDestTemp - 50 && posX < xDestTemp + 50 && posY > yDestTemp - 50 && posY < yDestTemp + 50))
                if (IsInRange(posX, xDestTemp, posY, yDestTemp, 50.0f))
                {
                    float xDestNext = 0.0f;
                    float yDestNext = 0.0f;
                    turningToNextPoint = true;

                    for (std::vector<WaypointNode>::iterator itr3 = itr->nodes.begin(); itr3 != itr->nodes.end(); ++itr3)
                    {
                        if (pointId + 1 == itr3->pointId)
                        {
                            xDestNext = itr3->x;
                            yDestNext = itr3->y;
                        }
                    }

                    // aantal ticks voor we nieuwe node bereiken (+50 / -50)
                    // nieuweDirection  62
                    // movingAngle     -99
                    // verschil tussen angle van NU en angle toekomstig
                    // movingAngle - nieuweDirection
                    // (nieuweDirection - movingAngle) / aantalTicks;
                    //float currMovingAngle = double(atan2(posY - yDest, xDest - posX) * 180 / M_PI);

                    //! TODO: Een beetje rare bug atm: nieuweDirection doet het op één hoek niet: de laatste. Daar is het resultaat ongeveer 155
                    //! terwijl het ongeveer -150 moet zijn (dus negatief).
                    float nieuweDirection = float(atan2(posY - yDestNext, xDestNext - posX) * 180 / M_PI);
                    float aantalGraden = nieuweDirection - movingAngle;
                    float aantalGradenPerTick = aantalGraden / 29;
                    //rotatingBodyAngle += 40;

                    //if (nieuweDirection > movingAngle)
                    //{
                    //    while (nieuweDirection > movingAngle)
                    //        movingAngle += 1;
                    //}
                    //else
                    //{
                    //    while (nieuweDirection < movingAngle)
                    //        movingAngle -= 1;
                    //}

                    //if (movingAngle >= aantalGraden)
                    //    turningToNextPoint = false;
                }
                else
                    turningToNextPoint = false;

                //! TODO: In plaats van lastPointIncreaseTime een integer genaamd lastPointTurned en verander op iedere nieuwe point turn.
                //! if (lastPointIncreaseTime != pointId)

                //between 3+ and 3-
                // als posX 198 is en xDest 200
                // 198 < 197 && 198 > 203 ||
                // 198 > 197 && 198 < 203
                // posX < xDest - 3 && posX > xDest + 3
                //if (pipeAngle < 138 && pipeAngle > 38) // hoger dan 38, lager dan 138
                //if (lastPointIncreaseTime == 0 && (posX == xDest && posY == yDest) || (posX < xDest - 2 && posX > xDest + 2) || (posX > xDest - 2 && posX < xDest + 2))
                //if ((posX == xDest && posY == yDest) || (posX < xDest - 6 && posX > xDest + 6 && posY < yDest - 6 && posY > yDest + 6) ||
                //                                        (posX > xDest - 6 && posX < xDest + 6 && posY > yDest - 6 && posY < yDest + 6))
                if ((posX == xDest && posY == yDest) || IsInRange(posX, xDest, posY, yDest, 6.0f))
                {
                    if (pointId == lastPointId)
                    {
                        if (itr->pickRandomAtEnd)
                            InitializeWaypoints(true);
                        else if (itr->repeat)
                        {
                            //incrItr = false;
                            //canMove = false;
                            itr->currDestPointId = 0;
                            //itr->nodes.begin();
                            //itr = waypoints.begin();

                            //itr->currDestPointId = itr->nodes.begin()->pointId;
                            xDest = itr->nodes.begin()->x;
                            yDest = itr->nodes.begin()->y;
                            //canMove = true;
                            //break;
                            //++itr;
                        }
                        else if (itr->repeatReversed)
                        {
                            /*incrItr = false;
                            canMove = false;
                            //xDest = itr->nodes.end()->x;
                            //yDest = itr->nodes.end()->y;

                            //std::reverse(itr->nodes.begin(), itr->nodes.end());
                            //std::reverse(waypoints.begin(), waypoints.end());
                            //itr->currDestPointId = lastPointId - 1;

                            std::vector<WaypointInformation> tempVector = waypoints;

                            for (std::vector<WaypointInformation>::reverse_iterator itr3 = waypoints.rbegin(); itr3 != waypoints.rend(); ++itr3)
                                tempVector.push_back(*itr3);

                            waypoints.clear();

                            for (std::vector<WaypointInformation>::iterator itr4 = tempVector.begin(); itr4 != tempVector.end(); ++itr4)
                                waypoints.push_back(*itr4);

                            break;
                            //itr = waypoints.begin();
                            //for (int i = 0; i <= lastPointId; ++i)
                            //    for (std::vector<WaypointNode>::iterator itr3 = itr->nodes.begin(); itr3 != itr->nodes.end(); ++itr3)
                            //        itr3->pointId = i;

                            //itr = waypoints.begin();
                            //break;*/
                        }
                    }
                    else
                    {
                        itr->currDestPointId++;
                        //++itr;
                    }

                    lastPointIncreaseTime = 200;
                }
            }

            if (incrItr)
                ++itr;
        }

        SDL_Rect npcNewRect = { Sint16(posX += float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD])), Sint16(posY -= float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD])), PLAYER_WIDTH, PLAYER_HEIGHT };
        bool foundCollision = false;

        std::vector<SDL_Rect2> wallRects = game->GetWalls();

        for (std::vector<SDL_Rect2>::iterator itr = wallRects.begin(); itr != wallRects.end(); ++itr)
        {
            if ((*itr).visible && WillCollision(&npcNewRect, &(*itr)))
            {
                CollisionSide collisionSide = GetSideOfCollision(&npcNewRect, &(*itr).GetNormalRect());
                foundCollision = true;

                //! Move up or down ONLY.
                if (collisionSide == SIDE_RIGHT || collisionSide == SIDE_LEFT)
                    posY += float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
                //! Move to left or right ONLY.
                else if (collisionSide == SIDE_BOTTOM || collisionSide == SIDE_TOP)
                    posX -= float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);

                break;
            }
        }

        //! Pushing an enemy away logic.
        {
            if (!foundCollision)
            {
                SDL_Rect currPlrRect = { Sint16(player->GetPosX()), Sint16(player->GetPosY()), PLAYER_WIDTH, PLAYER_HEIGHT };

                if (WillCollision(&rectBody, &currPlrRect))
                {
                    foundCollision = true; //! Maybe useful in the future - who knows..

                    //? TODO: Take mass in consideration (use some alg. or not?)
                    float _newX = player->GetPosX() + float(cos(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
                    float _newY = player->GetPosY() - float(sin(movingAngle * M_PI / 180.0) * moveSpeed[MOVE_TYPE_FORWARD]);
                    SDL_Rect newNpcRect = { Sint16(_newX), Sint16(_newY), PLAYER_WIDTH, PLAYER_HEIGHT };

                    for (std::vector<SDL_Rect2>::iterator itrWall = wallRects.begin(); itrWall != wallRects.end(); ++itrWall)
                    {
                        if ((*itrWall).visible && WillCollision(&newNpcRect, &(*itrWall)))
                        {
                            CollisionSide collisionSide = GetSideOfCollision(&newNpcRect, &(*itrWall).GetNormalRect());

                            //! Move up or down ONLY.
                            if (collisionSide == SIDE_RIGHT || collisionSide == SIDE_LEFT)
                                _newX = player->GetPosX();
                            //! Move to left or right ONLY.
                            else if (collisionSide == SIDE_BOTTOM || collisionSide == SIDE_TOP)
                                _newY = player->GetPosY();

                            //! TODO: This will fix one issue but bring up another. If we are currently pushing the player into a corner and we only iterate over the first
                            //! wall found in the iteration the player collides with and do this over and over, we will be pushed outside the game (through the walls).
                            //! However, if we do NOT have this break here we will collide with more than one wall which often means we stop moving because both the _newX
                            //! and the _newY variables are set to not be changed as we, for example, hit the upper side of one wall but the left side of another.
                            break;
                        }
                    }

                    player->SetPosX(_newX);
                    player->SetPosY(_newY);
                }
            }
        }

        rectBody.x = Sint16(posX);
        rectBody.y = Sint16(posY);

        rectPipe.x = Sint16(posX);
        rectPipe.y = Sint16(posY);

        rotatingPipeAngle += 3;
        rotatedBody = rotozoomSurface(bodySprite, movingAngle, 1.0, 0);
        rotatedPipe = rotozoomSurface(pipeSprite, rotatingPipeAngle, 1.0, 0);

        rectPipe.x -= rotatedPipe->w / 2 - pipeSprite->w / 2;
        rectPipe.y -= rotatedPipe->h / 2 - pipeSprite->h / 2;

        rectBody.x -= rotatedBody->w / 2 - bodySprite->w / 2;
        rectBody.y -= rotatedBody->h / 2 - bodySprite->h / 2;
    }
    else
    {
        rectBody.x = Sint16(posX);
        rectBody.y = Sint16(posY);
        rectPipe.x = Sint16(posX);
        rectPipe.y = Sint16(posY);
        rotatingBodyAngle += 3.0f;
        rotatedBody = rotozoomSurface(bodySprite, rotatingBodyAngle, 1.0, 0);
        rectBody.x -= rotatedBody->w / 2 - bodySprite->w / 2;
        rectBody.y -= rotatedBody->h / 2 - bodySprite->h / 2;
    }
}

void Enemy::HandleTimers(unsigned int diff_time)
{
    if (isDead)
        return;

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
    else
    {
        if (diff_time >= randomShootTimer)
            randomShootTimer = 0;
        else
            randomShootTimer -= diff_time;
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

    if (lastPointIncreaseTime > 0)
    {
        if (diff_time >= lastPointIncreaseTime)
            lastPointIncreaseTime = 0;
        else
            lastPointIncreaseTime -= diff_time;
    }
}

void Enemy::SetRectPosX(Sint16 val, bool body, bool pipe)
{
    if (body)
        rectBody.x = val;

    if (pipe)
        rectPipe.x = val;
}

void Enemy::SetRectPosY(Sint16 val, bool body, bool pipe)
{
    if (body)
        rectBody.y = val;

    if (pipe)
        rectPipe.y = val;
}

void Enemy::InitializeWaypoints(bool eraseCurrent /* = false */)
{
    if (isDead)
        return;

    if (eraseCurrent)
        waypoints.clear();

    WaypointInformation wpInfo;
    wpInfo.pickRandomAtEnd = true;
    wpInfo.repeat = false;
    wpInfo.repeatReversed = false;
    wpInfo.xVelocity = moveSpeed[MOVE_TYPE_FORWARD];
    wpInfo.yVelocity = moveSpeed[MOVE_TYPE_FORWARD];
    wpInfo.currDestPointId = 0;
    int rand = urand(0, 2);

    for (int i = 0; i < 3; ++i)
    {
        WaypointNode node;
        node.pointId = i;

        switch (rand)
        {
            case 0:
            {
                switch (i)
                {
                    case 0:
                        node.x = 270.0f;
                        node.y = 480.0f;
                        break;
                    case 1:
                        node.x = 400.0f;
                        node.y = 200.0f;
                        break;
                    //case 2:
                    //    node.x = 560.0f;
                    //    node.y = 130.0f;
                    //    break;
                    //case 3:
                    //    node.x = 770.0f;
                    //    node.y = 350.0f;
                    //    break;
                    //case 4:
                    //    node.x = 212.0f;
                    //    node.y = 480.0f;
                    //    break;
                    //case 5:
                    //    node.x = 100.0f;
                    //    node.y = 450.0f;
                    //    break;
                    //case 6:
                    //    node.x = 100.0f;
                    //    node.y = 84.0f;
                    //    break;
                    //case 7:
                    //    node.x = 278.0f;
                    //    node.y = 67.0f;
                    //    break;
                    case 2:
                        node.x = 140.0f;
                        node.y = 330.0f;
                        break;
                    default:
                        return; //! Als we hier door zouden gaan komen er waypoints zonder X en Y as en dat willen we niet.
                }
                break;
            }
            case 1:
            {
                switch (i)
                {
                    case 0:
                        node.x = 820.0f;
                        node.y = 460.0f;
                        break;
                    case 1:
                        node.x = 370.0f;
                        node.y = 80.0f;
                        break;
                    case 2:
                        node.x = 90.0f;
                        node.y = 460.0f;
                        break;
                    default:
                        return; //! Als we hier door zouden gaan komen er waypoints zonder X en Y as en dat willen we niet.
                }
                break;
            }
            case 2:
            {
                switch (i)
                {
                    case 0:
                        node.x = 65.0f;
                        node.y = 490.0f;
                        break;
                    case 1:
                        node.x = 140.0f;
                        node.y = 85.0f;
                        break;
                    case 2:
                        node.x = 575.0f;
                        node.y = 95.0f;
                        break;
                    default:
                        return; //! Als we hier door zouden gaan komen er waypoints zonder X en Y as en dat willen we niet.
                }
                break;
            }
        }

        //if (node.x == 0.0f || node.y == 0.0f)
        //    continue;

        wpInfo.nodes.push_back(node);
    }

    waypoints.push_back(wpInfo);
}

void Enemy::AddWaypointPath(WaypointInformation wpInfo)
{
    return;
}

void Enemy::JustDied()
{
    isDead = true;
    game->AddBigExplosion(posX, posY, 0, 80);
}
