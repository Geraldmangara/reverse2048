#ifndef GRIDGAME_H_INCLUDED
#define GRIDGAME_H_INCLUDED
#include <iostream>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <random>
#include <string>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
using namespace std;


// Common Position struct to be used by both GridGame and AIs
struct Position
{
    int row, col;
    bool operator==(const Position& other) const
    {
        return row == other.row && col == other.col;
    }
    bool operator!=(const Position& other) const
    {
        return !(*this == other);
    }
};
// Declaration of AI classes
class ExpectimaxAI;
class SmartMergeMax;
class GridGame
{
private:
    // Constants for grid setup
    const vector<int> VALID_NUMBERS = {128, 256, 512};
    const int MIN_GRID_SIZE = 3;
    const int MAX_GRID_SIZE = 5;
    const int EMPTY = -1;
    const int WIN_VALUE = 2;




    // Game state variables

    int gridSize;
    int currentNumber;
    int Ai1Count;
    int Ai2Count;



    vector<vector<int>> grid1, grid2; // Two separate game boards



    mt19937 rng;// Random number generator


    vector<int> possibleSpawnValues; // Store the possible values that can spawn


    // Tracks positions on each grid
    Position pos1, pos2;

    // AI for both grids
    SmartMergeMax* ai1;    // SmartMergeMax AI for grid1
    ExpectimaxAI* ai2;     // ExpectimaxAI for grid2


    // Game state tracking
    bool grid1GameOver;
    bool grid2GameOver;

    char lastMoveGrid1;    // Last move direction for grid1
    char lastMoveGrid2;    // Last move direction for grid2

    bool isFirstDisplay;   // Track if it's the initial board display

    // Initialize possible spawn values based on the starting number
    void initPossibleSpawnValues();

    // Checks if config file values are acceptable
    void validateConfiguration() const;

    // Prepares both grids and adds two numbers each
    void initializeGrids();

    // Figures out where the cursor would move given a direction
    Position calculateNewPosition(Position pos, char dir) const;

    // Checks if a given grid position is within bounds
    bool isValidPosition(Position pos) const;

    // Adds a new tile with a random value from possibleSpawnValues in an empty spot
    void spawnRandomNumber(vector<vector<int>>& grid);

    // Moves tiles in a given direction and handles merging
    bool processMovement(Position& pos, vector<vector<int>>& grid, char dir);

    // Checks if the game has reached a win or stalemate for a grid
    bool checkGameOver(const vector<vector<int>>& grid) const;

public:
    // Constructor that loads config and starts game
    GridGame(const string& InputFile = "reverse2048.txt");

    // Destructor to clean up the AIs
    ~GridGame();

    // Process user input
    void handleInput(char input);

    // Prints out both grids side-by-side with current number
    void displayGameState() const;

    // Shows movement keys to the player
    void showControls() const;

    // Let both AIs play one step
    bool aiPlayOneStep();

    // Let the AI1 (SmartMergeMax) play one step
    bool ai1PlayOneStep();

    // Let the AI2 (Expectimax) play one step
    bool ai2PlayOneStep();

    // Let the AIs play until game over
    void aiPlayUntilGameOver();

    // Starts the main game loop
    void run();

    // Getter method for processMovement to be accessible by the AIs
    bool performProcessMovement(Position& pos, vector<vector<int>>& grid, char dir);

    // Direction character to string for display
    string directionToString(char dir) const;

    // checks if one of the AI has won
    bool hasWon(const vector<vector<int>>& grid)const;
};
#endif // GRIDGAME_H_INCLUDED
