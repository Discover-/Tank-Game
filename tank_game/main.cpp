#include "game.h"

int main (int argc, char* args[])
{
    Game* game = new Game();
    return game->Update();
}
