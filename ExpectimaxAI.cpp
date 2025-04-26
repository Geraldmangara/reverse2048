#include "ExpectimaxAI.h"
#include "GridGame.h"

// Implementation of the playOneStep method
bool ExpectimaxAI::playOneStep(GridGame* game) {
    char bestMove = getBestMove();
    if (bestMove != 'n') {
        game->handleInput(bestMove);
        return true;
    }
    return false;
}
