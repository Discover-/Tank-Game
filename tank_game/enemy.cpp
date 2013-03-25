#include "game.h"

Enemy::Enemy(Game* _game, float x, float y, SDL_Surface* body, SDL_Surface* pipe, SDL_Rect _rectBody, SDL_Rect _rectPipe)
{
    if (!_game)
        return;

    posX = x;
    posY = y;
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
}

bool forward = true;
int counter = 0;

void Enemy::Update()
{
    /*if (forward)
    {
        posX += float(cos(movingAngle * M_PI / 180.0) * MOVES_SPEED_FORWARD);
        posY -= float(sin(movingAngle * M_PI / 180.0) * MOVES_SPEED_FORWARD);

        rotatingPipeAngle = -rotatingPipeAngle;
        rotatedPipe = rotozoomSurface(pipeSprite, rotatingPipeAngle, 1.0, 0);
    }
    else
    {
        posX -= float(cos(movingAngle * M_PI / 180.0) * MOVES_SPEED_BACKWARD);
        posY += float(sin(movingAngle * M_PI / 180.0) * MOVES_SPEED_BACKWARD);

        rotatingPipeAngle = 180 - rotatingPipeAngle;
        rotatedPipe = rotozoomSurface(pipeSprite, rotatingPipeAngle, 1.0, 0);
    }

    //movingAngle += 3;

    if (posX < 20 && !forward)
        forward = true;

    if (posX > 880 && forward)
        forward = false;*/

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

            bool canMove = true;

            //if (!turningToNextPoint)
                movingAngle = float(atan2(posY - yDest, xDest - posX) * 180 / M_PI);

            if ((posX < xDestTemp - 50 && posX > xDestTemp + 50 && posY < yDestTemp - 50 && posY > yDestTemp + 50) ||
                (posX > xDestTemp - 50 && posX < xDestTemp + 50 && posY > yDestTemp - 50 && posY < yDestTemp + 50))
            {
                counter++;
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
                // nieuweDirection
                // movingAngle += ;
                // verschil tussen angle van NU en angle toekomstig
                // movingAngle - nieuweDirection
                // (nieuweDirection - movingAngle) / aantalTicks;
                //float currMovingAngle = double(atan2(posY - yDest, xDest - posX) * 180 / M_PI);

                //! TODO: Een beejte rare bug atm: nieuweDirection doet het op één hoek niet: de laatste. Daar is het resultaat 155 terwijl het
                //! ongeveer -155 moet zijn (dus negatief).
                float nieuweDirection = float(atan2(posY - yDestNext, xDestNext - posX) * 180 / M_PI);
                float aantalGraden = nieuweDirection - movingAngle;
                float aantalGradenPerTick = aantalGraden / 29;
                movingAngle += 30;

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
            if ((posX == xDest && posY == yDest) || (posX < xDest - 6 && posX > xDest + 6 && posY < yDest - 6 && posY > yDest + 6) ||
                                                    (posX > xDest - 6 && posX < xDest + 6 && posY > yDest - 6 && posY < yDest + 6))
            {
                if (pointId == lastPointId)
                {
                    if (itr->repeat)
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
                    canMove = true;
                    //++itr;
                }

                lastPointIncreaseTime = 200;
            }

            if (canMove)
            {
                //movingAngle = double(atan2(posY - yDest, xDest - posX) * 180 / M_PI);
                _rotatingAngle += 3;
                posX += float(cos(movingAngle * M_PI / 180.0) * itr->xVelocity);
                posY -= float(sin(movingAngle * M_PI / 180.0) * itr->yVelocity);
                rectBody.x = Sint16(posX);
                rectBody.y = Sint16(posY);

                SDL_Surface* rotatedBodyEnemy = rotozoomSurface(bodySprite, movingAngle, 1.0, 0);
                SDL_Surface* rotatedPipeEnemy = rotozoomSurface(pipeSprite, _rotatingAngle, 1.0, 0);

                rotatedBody = rotozoomSurface(bodySprite, movingAngle, 1.0, 0);
                rotatedPipe = rotozoomSurface(pipeSprite, _rotatingAngle, 1.0, 0);

                rectPipe.x = Sint16(posX);
                rectPipe.y = Sint16(posY);
                rectPipe.w = PLAYER_WIDTH;
                rectPipe.h = PLAYER_HEIGHT;

                rectPipe.x -= rotatedPipe->w / 2 - pipeSprite->w / 2;
                rectPipe.y -= rotatedPipe->h / 2 - pipeSprite->h / 2;

                rectBody.x -= rotatedBody->w / 2 - bodySprite->w / 2;
                rectBody.y -= rotatedBody->h / 2 - bodySprite->h / 2;

                SetRotatedInfo(rotatedBodyEnemy, rotatedPipeEnemy, bodySprite, pipeSprite);

                //++itr;
            }
        }

        if (incrItr)
            ++itr;
    }

    if (posX < 0)
        posX = 0;

    if (posY < 0)
        posY = 0;

    if (posX > 938)
        posX = 938;

    if (posY > 544)
        posY = 544;

    lastMovingAngle = movingAngle;
}

void Enemy::HandleTimers(unsigned int diff_time)
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

void Enemy::InitializeWaypoints()
{
    WaypointInformation wpInfo;
    wpInfo.repeat = true;
    wpInfo.repeatReversed = false;
    wpInfo.xVelocity = 2.0f;
    wpInfo.yVelocity = 2.0f;
    wpInfo.currDestPointId = 0;

    for (int i = 0; i < 4; ++i)
    {
        WaypointNode node;
        node.pointId = i;

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
            case 2:
                node.x = 560.0f;
                node.y = 130.0f;
                break;
            case 3:
                node.x = 770.0f;
                node.y = 350.0f;
                break;
            //case 4:
            //    node.x = 500.0f;
            //    node.y = 100.0f;
            //    break;
            //case 5:
            //    node.x = 200.0f;
            //    node.y = 200.0f;
            //    break;
            //case 6:
            //    node.x = 300.0f;
            //    node.y = 300.0f;
            //    break;
            //case 7:
            //    node.x = 40.0f;
            //    node.y = 10.0f;
            //    break;
            //case 8:
            //    node.x = 400.0f;
            //    node.y = 100.0f;
            //    break;
            default:
                break;
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
