#ifndef EXPECTIMAXAI_H_INCLUDED
#define EXPECTIMAXAI_H_INCLUDED

#include "GridGame.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cfloat>
#include <unordered_map>
#include <cmath>

using namespace std;

class GridGame;

class ExpectimaxAI
{
private:
    vector<vector<int>>& grid;       // Reference to the game grid
    Position& position;              // Reference to the player position
    const int gridSize, maxDepth, EMPTY; // Grid dimensions, search depth, and empty cell value
    int startNumber;                 // Starting number for tile generation
    vector<int> possibleSpawnValues; // Values that can spawn on the grid
    unordered_map<string, double> evalCache; // Cache for grid evaluations
    // Decay parameters
    const double decayFactor = 0.1;  // Controls how quickly the weight decreases

    // Direction vector structure for movement processing
    struct DirVector
    {
        int rowDelta, colDelta;      // Movement direction
        int startRow, endRow, rowStep; // Row iteration parameters
        int startCol, endCol, colStep; // Column iteration parameters
    };
    unordered_map<char, DirVector> dirVectors; // Maps direction keys to vectors

    // Converts grid to string representation for caching
    string gridToString(const vector<vector<int>>& g) const;

    // Checks if a position is within grid boundaries
    bool isValidPosition(int row, int col) const;

    // Initializes direction vectors for all possible moves
    void initDirectionVectors();

    // Initializes possible spawn values based on startNumber
    void initPossibleSpawnValues();

    // Calculates position weight based on distance from corner
    double getPositionWeight(int row, int col) const;

    // Checks if a move in the given direction is possible
    bool tryMove(const vector<vector<int>>& g, char dir) const;

    // Simulates a move in the given direction and returns new grid
    vector<vector<int>> simulateMove(const vector<vector<int>>& g, char dir) const;

    // Returns a list of all empty cell positions
    vector<Position> getEmptyCells(const vector<vector<int>>& g) const;

    // Checks if the grid contains the winning value of 1
    bool hasValueOne(const vector<vector<int>>& g) const;

    // Determines if the game is over (won or no moves possible)
    bool checkGameOver(const vector<vector<int>>& g) const;

    // Evaluates grid state and returns a score
    double evaluateGrid(const vector<vector<int>>& g) const;

    // Implements the expectimax algorithm for decision making
    double expectimax(const vector<vector<int>>& g, int depth, bool isMaxPlayer);

public:
    // Constructor initializes the AI with game parameters
    ExpectimaxAI(vector<vector<int>>& g, Position& pos, int size,
                 int initialNumber, int depth, int empty);

    // Customizes the decay factor for position weighting
    void setDecayFactor(double factor);

    // Updates spawn values when the game configuration changes
    void updateSpawnValues(int newStartNumber);

    // Determines and returns the best move direction
    char getBestMove();

    // Clears the evaluation cache
    void resetCache();

    // Executes one AI move in the game
    bool playOneStep(GridGame* game);
};

#endif // EXPECTIMAXAI_H_INCLUDED
