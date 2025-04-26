/** reverse2048
 * Grid Game - reverse2048 class with two grids
 *
 * This program implements a puzzle game where players(algorithms) control two separate grids,
 * trying to merge numbers to reach the value 2. The game mechanics are similar to 2048
 * but with division instead of multiplication when merging.
 */
#include "GridGame.h"

// Entry point of the program
int main() {
    try {
        GridGame game;
        game.run();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
