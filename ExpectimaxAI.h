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
    vector<vector<int>>& grid;       
    Position& position;              // Reference to the player position
    const int gridSize, maxDepth, EMPTY; // Grid dimensions, search depth, and empty cell value
    int startNumber;                 
    vector<int> possibleSpawnValues; // Values that can spawn on the grid
    unordered_map<string, double> evalCache; // Cache for grid evaluations
    const double decayFactor = 0.1;  // Controls how quickly the weight decreases
   
    struct DirVector// movement vector
    {
        int rowDelta, colDelta;      
        int startRow, endRow, rowStep; 
        int startCol, endCol, colStep; 
    };
    unordered_map<char, DirVector> dirVectors; 

    // Converts grid to string representation for caching
    string gridToString(const vector<vector<int>>& g) const;

    // Checks if a position is within grid boundaries
    bool isValidPosition(int row, int col) const;

    //Initialises direction vectors for all possible moves
    void initDirectionVectors();
    void initPossibleSpawnValues();

    // Gets the weighted value of all the non-zero 
    double getPositionWeight(int row, int col) const;

    // Checks if a move in the given direction is possible
    bool tryMove(const vector<vector<int>>& g, char dir) const;
    vector<vector<int>> simulateMove(const vector<vector<int>>& g, char dir) const;
    vector<Position> getEmptyCells(const vector<vector<int>>& g) const;

    // Checks if the grid contains the winning value of 2
    bool hasValueTwo(const vector<vector<int>>& g) const;
    bool checkGameOver(const vector<vector<int>>& g) const;
    double evaluateGrid(const vector<vector<int>>& g) const;

    //Implements the recursion
    double expectimax(const vector<vector<int>>& g, int depth, bool isMaxPlayer);

public:
    // Constructor initialises the AI with game parameters
    ExpectimaxAI(vector<vector<int>>& g, Position& pos, int size,
                 int initialNumber, int depth, int empty);

    //Customises the decay factor for position weighting
    void setDecayFactor(double factor);
    void updateSpawnValues(int newStartNumber);
    char getBestMove(); 
    void resetCache();// Clears the evaluation cache

    // Executes one AI move in the game
    bool playOneStep(GridGame* game);
};

#endif // EXPECTIMAXAI_H_INCLUDED
