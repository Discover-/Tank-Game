#include "game.h"

Game::Game()
{
    pipeAngle = 0;
    player = NULL;
    lastMouseX = 0;
    lastMouseY = 0;
}

Game::~Game()
{

}

void Game::ApplySurface(int x, int y, SDL_Surface* source, SDL_Surface* destination)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SDL_BlitSurface(source, NULL, destination, &rect);
}

void Game::DrawSprite(int srcX, int srcY, int dstX, int dstY, int width, int height, SDL_Surface* source, SDL_Surface* destination)
{
    SDL_Rect src;
    src.x = srcX;
    src.y = srcY;
    src.w = width;
    src.h = height;

    SDL_Rect dst;
    dst.x = dstX;
    dst.y = dstY;
    dst.w = width;
    dst.h = height;

    SDL_BlitSurface(source, &src, destination, &dst);
}

void Game::StoreSurfaceByTime(char* bmpFile, SDL_Rect rect, RGB rgb, unsigned int despawnTime)
{
    TemporarilySurfaces tmpSurface;
    tmpSurface.bmpFile = bmpFile;
    tmpSurface.despawnTime = despawnTime;
    tmpSurface.rect = rect;
    tmpSurface.rgb = rgb;
    tmpSurface.surface = NULL;
    temporarilySurfaces.push_back(tmpSurface);
}

void Game::HandleTimers(unsigned int diff_time)
{
    if (!temporarilySurfaces.empty())
    {
        for (std::vector<TemporarilySurfaces>::iterator itr = temporarilySurfaces.begin(); itr != temporarilySurfaces.end(); )
        {
            if (itr->despawnTime > 0)
            {
                if (diff_time >= itr->despawnTime)
                    itr->despawnTime = 0;
                else
                    itr->despawnTime -= diff_time;

                ++itr;
            }
            else
            {
                //SDL_FreeSurface(itr->);
                temporarilySurfaces.erase(itr++);
                itr = temporarilySurfaces.begin();
            }
        }
    }

    if (!mineExplosions.empty())
    {
        for (std::vector<MineExplosions>::iterator itr = mineExplosions.begin(); itr != mineExplosions.end(); )
        {
            if (itr->delay > 0 && itr->frame < 7)
            {
                if (diff_time >= itr->delay)
                {
                    itr->delay = 100;
                    itr->frame++;
                }
                else
                    itr->delay -= diff_time;

                ++itr;
            }
            else
            {
                mineExplosions.erase(itr++);
                itr = mineExplosions.begin();
            }
        }
    }

    if (player)
        player->HandleTimers(diff_time);

    for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
        if ((*itr)->IsAlive())
            (*itr)->HandleTimers(diff_time);
}

void Game::BlitSurface(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect, RGB rgb)
{
    SDL_BlitSurface(src, srcrect, dst, dstrect);
    SDL_SetColorKey(src, SDL_SRCCOLORKEY, SDL_MapRGB(src->format, rgb.r, rgb.g, rgb.b));
}

void Game::AddWall(Sint16 x, Sint16 y, Sint16 w /* = 50 */, Sint16 h /* = 50 */, bool breakable /* = false */, bool visible /* = true */)
{
    SDL_Rect2 wall;
    wall.x = x;
    wall.y = y;
    wall.w = w;
    wall.h = h;
    wall.visible = visible;
    wall.breakable = breakable;
    wallRectangles.push_back(wall);
}

void Game::InitializeWalls()
{
    Sint16 currWallX = 0, currWallY = 0;
    AddWall(0, 0);

    for (int i = 1; i < 80; ++i)
    {
        if (i < 20 && i > 0) //! First horizontal wall (starts left upper corner - ends right upper corner)
            currWallX += 50;
        else if (i < 40 && i > 20) //! First vertical wall (starts right upper corner - ends right bottom corner)
        {
            currWallX = 0;
            currWallY += 50;
        }
        else if (i < 60 && i > 40) //! Second horizontal wall (starts left upper corner - ends left bottom corner)
        {
            currWallX += 50;
            currWallY = 550;
        }
        else if (i < 80 && i > 60) //! Second vertical wall (starts left bottom corner - ends right bottom corner)
        {
            //! Reset Y co-ordinate for this line of walls on first wall placement because it starts from 0.
            if (i == 61)
                currWallY = 0;

            currWallX = 950;
            currWallY += 50;
        }

        AddWall(currWallX, currWallY);
    }

    currWallX = 225;
    currWallY = 150;

    //! A special line of walls in the left middle-ish part of the field.
    for (int i = 80; i < 86; ++i)
    {
        AddWall(currWallX, currWallY, 50, 50, (i == 82 || i == 83)); //! Walls 82 and 83 are breakable by landmines.
        currWallY += 50;
    }
}

void Game::InitializeCharacters(SDL_Surface* spriteBodyPlr, SDL_Surface* spritePipePlr, SDL_Surface* spriteBodyNpc, SDL_Surface* spritePipeNpc)
{
    player = new Player(this, 400.0f, 200.0f);

    SDL_Rect npcRect1 = { 300, 300, PLAYER_WIDTH, PLAYER_HEIGHT };
    SDL_Rect npcRect2 = { 300, 400, PLAYER_WIDTH, PLAYER_HEIGHT };
    SDL_Rect npcRect3 = { 300, 500, PLAYER_WIDTH, PLAYER_HEIGHT };
    Enemy* enemy1 = new Enemy(this, 300.0f, 300.0f, spriteBodyNpc, spritePipeNpc, npcRect1, npcRect1);
    Enemy* enemy2 = new Enemy(this, 350.0f, 350.0f, spriteBodyNpc, spritePipeNpc, npcRect2, npcRect2);
    Enemy* enemy3 = new Enemy(this, 400.0f, 400.0f, spriteBodyNpc, spritePipeNpc, npcRect3, npcRect3);
    enemies.push_back(enemy1);
    enemies.push_back(enemy2);
    enemies.push_back(enemy3);

    for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
        (*itr)->InitializeWaypoints();
}

int Game::Update()
{
    isRunning = true;

    SDL_Init(SDL_INIT_EVERYTHING);
    //SDLNet_Init();
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    
    SDL_Surface* tmpBodyPlr = SDL_LoadBMP("sprite_body.bmp");
    SDL_Surface* spriteBodyPlr = SDL_DisplayFormat(tmpBodyPlr);
    SDL_FreeSurface(tmpBodyPlr);
    SDL_SetColorKey(spriteBodyPlr, SDL_SRCCOLORKEY, COLOR_WHITE);
    
    SDL_Surface* tmpPipePlr = SDL_LoadBMP("sprite_pipe.bmp");
    SDL_Surface* spritePipePlr = SDL_DisplayFormat(tmpPipePlr);
    SDL_FreeSurface(tmpPipePlr);
    SDL_SetColorKey(spritePipePlr, SDL_SRCCOLORKEY, COLOR_WHITE);
    
    SDL_Surface* tmpBodyNpc = SDL_LoadBMP("sprite_body_npc.bmp");
    SDL_Surface* spriteBodyNpc = SDL_DisplayFormat(tmpBodyNpc);
    SDL_FreeSurface(tmpBodyNpc);
    SDL_SetColorKey(spriteBodyNpc, SDL_SRCCOLORKEY, COLOR_WHITE);
    
    SDL_Surface* tmpPipeNpc = SDL_LoadBMP("sprite_pipe_npc.bmp");
    SDL_Surface* spritePipeNpc = SDL_DisplayFormat(tmpPipeNpc);
    SDL_FreeSurface(tmpPipeNpc);
    SDL_SetColorKey(spritePipeNpc, SDL_SRCCOLORKEY, COLOR_WHITE);
    
    SDL_Surface* tmpWall = SDL_LoadBMP("wall.bmp");
    SDL_Surface* wall = SDL_DisplayFormat(tmpWall);
    SDL_FreeSurface(tmpWall);
    //SDL_SetColorKey(wall, SDL_SRCCOLORKEY, COLOR_WHITE);
    
    SDL_Surface* tmpWallBreakable = SDL_LoadBMP("wall_breakable.bmp");
    SDL_Surface* wallBreakable = SDL_DisplayFormat(tmpWallBreakable);
    SDL_FreeSurface(tmpWallBreakable);
    SDL_SetColorKey(wallBreakable, SDL_SRCCOLORKEY, COLOR_WHITE);
    
    SDL_Surface* tmpSlowArea = SDL_LoadBMP("slow_area.bmp");
    SDL_Surface* slowArea = SDL_DisplayFormat(tmpSlowArea);
    SDL_FreeSurface(tmpSlowArea);
    SDL_SetColorKey(slowArea, SDL_SRCCOLORKEY, COLOR_WHITE);

    InitializeWalls();
    InitializeCharacters(spriteBodyPlr, spritePipePlr, spriteBodyNpc, spritePipeNpc);

    SDL_Rect slowAreaRect = { 200, 70, 150, 75 };
    slowAreaRectangles.push_back(slowAreaRect);

    SDL_Surface* rotatedBodyPlr = rotozoomSurface(spriteBodyPlr, 0.0f, 1.0, 0);
    SDL_Surface* rotatedPipePlr = rotozoomSurface(spritePipePlr, 0.0f, 1.0, 0);
    SDL_Surface* rotatedBodyNpc = rotozoomSurface(spriteBodyNpc, 0.0f, 1.0, 0);
    SDL_Surface* rotatedPipeNpc = rotozoomSurface(spritePipeNpc, 0.0f, 1.0, 0);

    for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
        (*itr)->SetRotatedInfo(rotatedBodyNpc, rotatedPipeNpc, spriteBodyNpc, spritePipeNpc);

    int mouseX = 10, mouseY = 10;

    //mineExplosionRect[0].x = 0;
    //mineExplosionRect[0].y = 0;
    //mineExplosionRect[0].w = 60;
    //mineExplosionRect[0].h = 60;

    //mineExplosionRect[1].x = 145;
    //mineExplosionRect[1].y = 0;
    //mineExplosionRect[1].w = 60;
    //mineExplosionRect[1].h = 60;

    int last_time = 0;
    int curr_time = 0;
    int diff_time = 0;

    //IPaddress ip;
    //SDLNet_ResolveHost(&ip, NULL, 12345);
    //TCPsocket server = SDLNet_TCP_Open(&ip);
    //TCPsocket client;
    //char* text = "Hello client!\n";

    unsigned int startTime = 0;
    Uint8* keystate = SDL_GetKeyState(NULL);

    while (isRunning && player)
    {
        SDL_FillRect(screen, NULL, COLOR_GREEN);

        startTime = SDL_GetTicks();

        //if (client = SDLNet_TCP_Accept(server))
        //{
        //    // communicate server <-> client
        //    SDLNet_TCP_Send(client, text, strlen(text) + 1);
        //    SDLNet_TCP_Close(client);
        //    break;
        //}

        while (SDL_PollEvent(&_event))
        {
            switch (_event.type)
            {
                case SDL_QUIT:
                    isRunning = false;
                    break;
                case SDL_MOUSEMOTION:
                    mouseX = _event.motion.x;
                    mouseY = _event.motion.y;
                    break;
                //! Om de één of andere reden werkt movement nogal slecht als het in de Player class handled is via Player::Update, ookal
                //! wordt die functie ook vanuit hier opgeroepen...
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    switch (_event.key.keysym.sym)
                    {
                        case SDLK_UP:
                        case SDLK_w:
                            player->SetKeysDown(0, _event.type == SDL_KEYDOWN);
                            break;
                        case SDLK_LEFT:
                        case SDLK_a:
                            player->SetKeysDown(1, _event.type == SDL_KEYDOWN);
                            break;
                        case SDLK_DOWN:
                        case SDLK_s:
                            player->SetKeysDown(2, _event.type == SDL_KEYDOWN);
                            break;
                        case SDLK_RIGHT:
                        case SDLK_d:
                            player->SetKeysDown(3, _event.type == SDL_KEYDOWN);
                            break;
                        case SDLK_SPACE:
                            if (player->GetLandmineCount() >= PLAYER_MAX_LANDMINES)
                                break;

                            if (player->CanPlaceLandmine())
                            {
                                if (Landmine* landmine = new Landmine(this, screen, SDL_LoadBMP("landmine.bmp"), float(player->GetPosX() + (PLAYER_WIDTH / 2) + 40), float(player->GetPosY() + (PLAYER_HEIGHT / 2)), LANDMINE_WIDTH, LANDMINE_HEIGHT))
                                {
                                    player->AddLandmine(landmine);
                                    allLandmines.push_back(landmine);
                                }

                                player->SetCanPlaceLandmine(false);
                                player->SetPlaceLandmineCooldown(500);
                                player->IncrLandmineCount();
                            }
                            break;
                        case SDLK_r:
                            //! Zet de speler terug naar de start positie als hij Shift + R indrukt en maak alle muren weer heel.
                            if (keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT])
                            {
                                player->SetPosX(400.0f);
                                player->SetPosY(200.0f);

                                for (std::vector<SDL_Rect2>::iterator itr = wallRectangles.begin(); itr != wallRectangles.end(); ++itr)
                                    if (!itr->visible)
                                        itr->visible = true;

                                for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
                                {
                                    if (!(*itr)->IsAlive())
                                        (*itr)->SetIsAlive(true);

                                    (*itr)->SetPosX((*itr)->GetStartPosX());
                                    (*itr)->SetPosY((*itr)->GetStartPosY());

                                    std::vector<WaypointInformation> waypoints = (*itr)->GetWaypoints();

                                    if (!waypoints.empty())
                                        (*itr)->InitializeWaypoints(true);
                                }

                                if (!allBullets.empty())
                                    for (std::vector<Bullet*>::iterator itr = allBullets.begin(); itr != allBullets.end(); ++itr)
                                        if (Bullet* bullet = (*itr))
                                            bullet->Explode(false);

                                if (!allLandmines.empty())
                                    for (std::vector<Landmine*>::iterator itr = allLandmines.begin(); itr != allLandmines.end(); ++itr)
                                        if (Landmine* landmine = (*itr))
                                            landmine->Explode(false);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                {
                    //! We plaatsen een landmijn wanneer er shift + muisklik gedaan wordt - dit is gewoon tijdelijk voor testen.
                    if (keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT])
                    {
                        if (Landmine* landmine = new Landmine(this, screen, SDL_LoadBMP("landmine.bmp"), _event.motion.x, _event.motion.y, LANDMINE_WIDTH, LANDMINE_HEIGHT))
                        {
                            player->AddLandmine(landmine);
                            allLandmines.push_back(landmine);
                        }

                        player->SetCanPlaceLandmine(false);
                        player->SetPlaceLandmineCooldown(500);
                        player->IncrLandmineCount();
                    }
                    else
                    {
                        if (player->GetBulletCount() >= PLAYER_MAX_BULLETS)
                            break;

                        if (player->CanShoot())
                        {
                            //! De 16 / 2 is de grootte en breedte van de muis.
                            float bulletX = float(player->GetPosX() + (PLAYER_WIDTH / 2) - 12) + (16 / 2);
                            float bulletY = float(player->GetPosY() + (PLAYER_HEIGHT / 2) - 12) + (16 / 2);

                            if (Bullet* bullet = new Bullet(this, screen, bulletX, bulletY, PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_BULLET_SPEED_X, PLAYER_BULLET_SPEED_Y, PLAYER_BULLET_LIFES, pipeAngle))
                            {
                                player->AddBullet(bullet);
                                allBullets.push_back(bullet);
                                //RGB smokeRGB;
                                //smokeRGB.r = 0xff;
                                //smokeRGB.g = 0xff;
                                //smokeRGB.b = 0xff;

                                //SDL_Rect smokeRect;
                                //smokeRect.x = Sint16(bulletX);
                                //smokeRect.y = Sint16(bulletY);

                                //StoreSurfaceByTime("smoke.bmp", smokeRect, smokeRGB);

                                player->SetCanShoot(false);
                                player->SetShootCooldown(200);
                                player->IncrBulletCount();
                            }
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

        if (!isRunning)
            break;

        curr_time = SDL_GetTicks();
        diff_time = curr_time - last_time;
        HandleTimers(curr_time - last_time);
        last_time = curr_time;

        player->Update();

        for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
            if ((*itr)->IsAlive())
                (*itr)->Update();

        float plrX = player->GetPosX();
        float plrY = player->GetPosY();
        float plrMovingAngle = player->GetMovingAngle();

        rotatedBodyPlr = rotozoomSurface(spriteBodyPlr, plrMovingAngle, 1.0, 0);

        SDL_Rect recBodyPlr = { int(plrX), int(plrY), 0, 0 };
        recBodyPlr.x -= rotatedBodyPlr->w / 2 - spriteBodyPlr->w / 2;
        recBodyPlr.y -= rotatedBodyPlr->h / 2 - spriteBodyPlr->h / 2;

        SDL_Rect recPipePlr = { int(plrX), int(plrY), PLAYER_WIDTH, PLAYER_HEIGHT };

        if (_event.type == SDL_MOUSEMOTION)
        //if (lastMouseX != _event.motion.x || lastMouseY != _event.motion.y)
        {
            pipeAngle = double(atan2((plrY + (PLAYER_HEIGHT / 2)) - _event.motion.y, _event.motion.x - (plrX + (PLAYER_WIDTH / 2))) * 180 / M_PI);
            rotatedPipePlr = rotozoomSurface(spritePipePlr, pipeAngle, 1.0, 0);
            lastMouseX = _event.motion.x;
            lastMouseY = _event.motion.y;
        }

        //! Hiermee zorgen we ervoor dat rotozoomSurface het midden van de pijp (het kanon) pakt als 'source' om vanaf te rotaten.
        recPipePlr.x -= rotatedPipePlr->w / 2 - spritePipePlr->w / 2;
        recPipePlr.y -= rotatedPipePlr->h / 2 - spritePipePlr->h / 2;

        //for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
        //{
        //    if ((*itr)->IsAlive())
        //    {
        //        (*itr)->GetRotatedBodyRect().x -= (*itr)->GetRotatedBodySurface()->w / 2 - spriteBodyNpc->w / 2;
        //        (*itr)->GetRotatedBodyRect().y -= (*itr)->GetRotatedBodySurface()->h / 2 - spriteBodyNpc->h / 2;
        //    }
        //}

        for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
        {
            if ((*itr)->IsAlive())
            {
                rotatedBodyNpc = (*itr)->GetRotatedBodySurface();
                rotatedPipeNpc = (*itr)->GetRotatedPipeSurface();
            }
        }

        if (!mineExplosions.empty())
        {
            for (std::vector<MineExplosions>::iterator itr = mineExplosions.begin(); itr != mineExplosions.end(); ++itr)
            {
                bool _continue = false;
                char* bmpFile = "";

                switch (itr->frame)
                {
                    case 0: bmpFile = "explosion_big_f0.bmp"; break;
                    case 1: bmpFile = "explosion_big_f1.bmp"; break;
                    case 2: bmpFile = "explosion_big_f2.bmp"; break;
                    case 3: bmpFile = "explosion_big_f3.bmp"; break;
                    case 4: bmpFile = "explosion_big_f4.bmp"; break;
                    case 5: bmpFile = "explosion_big_f5.bmp"; break;
                    case 6: bmpFile = "explosion_big_f6.bmp"; break;
                    default: _continue = true; break;
                }

                if (_continue)
                    continue;

                SDL_Rect tmpRectt;
                tmpRectt.x = Sint16(itr->x);
                tmpRectt.y = Sint16(itr->y);

                RGB explosionRGB;
                explosionRGB.r = 0x00;
                explosionRGB.g = 0x00;
                explosionRGB.b = 0x00;

                StoreSurfaceByTime(bmpFile, tmpRectt, explosionRGB, 80);

                //SDL_Surface* tmpMineExpl = SDL_LoadBMP(bmpFile);
                //SDL_Surface* mineExpl = SDL_DisplayFormat(tmpMineExpl);
                //SDL_FreeSurface(tmpMineExpl);
                //SDL_SetColorKey(mineExpl, SDL_SRCCOLORKEY, COLOR_WHITE);
                //SDL_BlitSurface(mineExpl, &mineExplosionRect[itr->frame], screen, &tmpRectt);

                //if (itr->frame < MAX_MINE_EXPLOSION_FRAMES)
                {
                    //itr->frame++;
                    //itr++;
                }
                //else
                {
                    //mineExplosions.erase(itr++);
                    //itr = mineExplosions.begin();
                }
            }
        }

        if (!temporarilySurfaces.empty())
        {
            for (std::vector<TemporarilySurfaces>::iterator itr = temporarilySurfaces.begin(); itr != temporarilySurfaces.end(); ++itr)
            {
                if (SDL_Surface* srfc = itr->surface)
                {
                    SDL_BlitSurface(srfc, NULL, screen, &itr->rect);

                    //! TODO: Deze colorkey call zou niet hoeven aangezien er per surface maar één oproep tot color key nodig is (tenzij je
                    //! de values wilt veranderen o.i.d.).
                    SDL_SetColorKey(srfc, SDL_SRCCOLORKEY, SDL_MapRGB(srfc->format, itr->rgb.r, itr->rgb.g, itr->rgb.b));
                }
                else
                {
                    SDL_Surface* tmpSurface = SDL_LoadBMP(itr->bmpFile);
                    SDL_SetColorKey(tmpSurface, SDL_SRCCOLORKEY, SDL_MapRGB(tmpSurface->format, itr->rgb.r, itr->rgb.g, itr->rgb.b));
                    SDL_BlitSurface(tmpSurface, NULL, screen, &itr->rect);
                    itr->surface = tmpSurface;
                }
            }
        }

        if (!allBullets.empty())
            for (std::vector<Bullet*>::iterator itr = allBullets.begin(); itr != allBullets.end(); ++itr)
                if (Bullet* bullet = (*itr))
                    bullet->Update();

        if (!allLandmines.empty())
            for (std::vector<Landmine*>::iterator itr = allLandmines.begin(); itr != allLandmines.end(); ++itr)
                if (Landmine* landmine = (*itr))
                    landmine->Update();

        for (std::vector<SDL_Rect2>::iterator itr = wallRectangles.begin(); itr != wallRectangles.end(); ++itr)
        {
            if (itr->visible)
            {
                SDL_Rect itrRect;
                itrRect.x = (*itr).x;
                itrRect.y = (*itr).y;
                itrRect.w = (*itr).w;
                itrRect.h = (*itr).h;
                SDL_BlitSurface((*itr).breakable ? wallBreakable : wall, NULL, screen, &itrRect);
            }
        }

        for (std::vector<SDL_Rect>::iterator itr = slowAreaRectangles.begin(); itr != slowAreaRectangles.end(); ++itr)
            SDL_BlitSurface(slowArea, NULL, screen, &(*itr));

        for (std::vector<Enemy*>::iterator itr = enemies.begin(); itr != enemies.end(); ++itr)
        {
            if ((*itr)->IsAlive())
            {
                SDL_Rect bodyRectEnemy = (*itr)->GetRotatedBodyRect();
                SDL_Rect pipeRectEnemy = (*itr)->GetRotatedPipeRect();
                SDL_Surface* rotatedBodyEnemy = (*itr)->GetRotatedBodySurface();
                SDL_Surface* rotatedPipeEnemy = (*itr)->GetRotatedPipeSurface();

                //! Make rotating of the surfaces properly centered.
                bodyRectEnemy.x -= rotatedBodyEnemy->w / 2 - rotatedBodyEnemy->w / 2;
                bodyRectEnemy.y -= rotatedBodyEnemy->h / 2 - rotatedBodyEnemy->h / 2;
                pipeRectEnemy.x -= rotatedPipeEnemy->w / 2 - rotatedPipeEnemy->w / 2;
                pipeRectEnemy.y -= rotatedPipeEnemy->h / 2 - rotatedPipeEnemy->h / 2;

                SDL_BlitSurface(rotatedBodyEnemy, NULL, screen, &bodyRectEnemy);
                SDL_BlitSurface(rotatedPipeEnemy, NULL, screen, &pipeRectEnemy);
            }
        }

        SDL_BlitSurface(rotatedBodyPlr, NULL, screen, &recBodyPlr);
        SDL_BlitSurface(rotatedPipePlr, NULL, screen, &recPipePlr);

        //DrawLine(screen, mouseX, mouseY, 300, 400);

        SDL_Flip(screen);

        if (SDL_GetTicks() - startTime < 1000 / FRAMES_PER_SECOND)
            SDL_Delay(1000 / FRAMES_PER_SECOND - (SDL_GetTicks() - startTime));

        char buff[255];
        sprintf_s(buff, "Tank Game   -   X: %f   -   Y: %f   -   pipeAngle: %f   -   Angle: %f   -   Mouse X: %u   -   Mouse Y: %u", plrX, plrY, pipeAngle, plrMovingAngle, _event.motion.x, _event.motion.y);
        SDL_WM_SetCaption(buff, NULL);
    }

    //for (std::vector<TemporarilySurfaces>::iterator itr = temporarilySurfaces.begin(); itr != temporarilySurfaces.end(); ++itr)
    //    if (SDL_Surface* surface = itr->surface)
    //        SDL_FreeSurface(surface);

    SDL_FreeSurface(spriteBodyPlr);
    SDL_FreeSurface(spritePipePlr);
    SDL_FreeSurface(spritePipeNpc);
    SDL_FreeSurface(rotatedBodyPlr);
    SDL_FreeSurface(rotatedPipePlr);
    SDL_FreeSurface(wall);
    SDL_FreeSurface(rotatedPipeNpc);
    SDL_FreeSurface(rotatedBodyNpc);
    //SDLNet_TCP_Close(server);
    //SDLNet_Quit();
    SDL_Quit(); //! 'Screen' wordt in SDL_Quit weggegooid.
    return 0;
}

void Game::UnregistrateBullet(Bullet* bullet)
{
    if (!allBullets.empty())
    {
        for (std::vector<Bullet*>::iterator itr = allBullets.begin(); itr != allBullets.end(); )
        {
            if ((*itr) == bullet)
            {
                allBullets.erase(itr);
                break;
            }
            else
                ++itr;
        }
    }

    //if (doRemove)
    {
        // breaks
        //if (bullet)
        //    if (SDL_Surface* surface = bullet->GetSurface())
        //        SDL_FreeSurface(surface);

        //bullet->~Bullet();
        //bullets.erase(_itr);
        //delete bullet; // freeze
    }
}

void Game::UnregistrateLandmine(Landmine* landmine)
{
    if (!allLandmines.empty())
    {
        for (std::vector<Landmine*>::iterator itr = allLandmines.begin(); itr != allLandmines.end(); )
        {
            if ((*itr) == landmine)
            {
                allLandmines.erase(itr);
                break;
            }
            else
                ++itr;
        }
    }

    //if (doRemove)
    {
        // breaks
        //if (landmine)
        //    if (SDL_Surface* surface = landmine->GetSurface())
        //        SDL_FreeSurface(surface);

        //landmine->~Landmine();
        //landmines.erase(_itr);
        //delete landmine; // freeze
    }
}

void PlacePixel(SDL_Surface* dest, int x, int y, int r, int g, int b)
{
    if (x >= 0 && x < dest->w && y >= 0 && y < dest->h)
        ((Uint32*)dest->pixels)[y * dest->pitch / 4 + x] = SDL_MapRGB(dest->format, r, g, b);
}

void SwapValues(int& a, int& b)
{
    int tmp = a;
    a = b;
    b = tmp;
}

void DrawLine(SDL_Surface* dest, int x0, int y0, int x1, int y1)
{
    bool step = abs(x1 - x0) < abs(y1 - y0);

    if (step)
    {
        SwapValues(x0, y0);
        SwapValues(x1, y1);
        //SwapValues(x0, x1);
        //SwapValues(y0, y1);
    }

    if (x1 < x0)
    {
        SwapValues(x0, x1);
        SwapValues(y0, y1);
    }

    float error = 0.0f;
    float roundError = float(abs(y1 - y0) / (x1 - x0));
    int y = 0;
    int ystep = (y1 > y0 ? 1 : -1);

    for (int i = 0; i < x1; ++i)
    {
        step ? PlacePixel(dest, y, i, 255, 255, 255) : PlacePixel(dest, i, y, 255, 255, 255);
        error += roundError;

        if (error >= 0.5)
        {
            y += ystep;
            error -= 1.0f;
        }
    }
}

void Game::RemoveBullet(Bullet* bullet)
{
    //for (std::vector<Bullet*>::iterator itr = allBullets.begin(); itr != allBullets.end(); ++itr)
    //{
    //    if ((*itr) == bullet)
    //    {
    //        allBullets.erase(itr);
    //        break;
    //    }
    //}
}

void Game::RemoveLandmine(Landmine* landmine)
{
    //for (std::vector<Landmine*>::iterator itr = allLandmines.begin(); itr != allLandmines.end(); ++itr)
    //{
    //    if ((*itr) == landmine)
    //    {
    //        allLandmines.erase(itr);
    //        break;
    //    }
    //}
}

bool Game::IsInSlowArea(float x, float y)
{
    for (std::vector<SDL_Rect>::iterator itr = slowAreaRectangles.begin(); itr != slowAreaRectangles.end(); ++itr)
    {
        float actualX = Sint16(itr->x - (itr->w / 2 - itr->w / 2));
        float actualY = Sint16(itr->y - (itr->h / 2 - itr->h / 2));
        float _actualX = x - (PLAYER_WIDTH / 2 - PLAYER_WIDTH / 2);
        float _actualY = y - (PLAYER_HEIGHT / 2 - PLAYER_HEIGHT / 2);
        //pipeRectEnemy.x -= rotatedPipeEnemy->w / 2 - rotatedPipeEnemy->w / 2;
        //pipeRectEnemy.y -= rotatedPipeEnemy->h / 2 - rotatedPipeEnemy->h / 2;

        if (IsInRange(_actualX, actualX, _actualY, actualY, 100.0f))
            return true;
    }

    return false;
}
