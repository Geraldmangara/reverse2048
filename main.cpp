/** reverse2048
 * Grid Game - reverse2048 class with two grids
 *
 * This program implements a puzzle game where players(algorithms) control two separate grids,
 * trying to merge numbers to reach the value 2. The game mechanics are similar to 2048
 * but with division instead of multiplication when merging.
 */


// Entry point of the program
#include "GridGame.h"
#include "GameRunner.h"
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        string configFile = "reverse2048.txt"; // Default config file

        // Allow command-line specification of config file
        if (argc > 1)
        {
            configFile = argv[1];
        }
     int currentNumber,gridSize;
        char s;
        
        while(currentNumber,gridSize){
        // Create and run the game
        GridGame game(currentNumber,gridSize);
        game.run();
        }
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    catch (...)
    {
        cerr << "Unknown error occurred." << endl;
        return 1;
    }



    // unit test stuff
    GameRunner runner;



    // Choose how many games to run
    int numGames = 100;

    // Test AI1 (SmartMergeMax)
    cout << "Running " << numGames << " games with SmartMergeMax AI..." << std::endl;
    runner.runAI1Games(numGames);

    // Test AI2 (ExpectimaxAI)
    cout << "\nRunning " << numGames << " games with ExpectimaxAI..." << std::endl;
    runner.runAI2Games(numGames);

    // Run competitions between both AIs
    cout << "\nRunning " << numGames << " competition games between both AIs..." << std::endl;
    runner.runCompetition(numGames);

    return 0;
}
