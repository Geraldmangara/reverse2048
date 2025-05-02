#ifndef SMARTMERGEMAX_H_INCLUDED
#define SMARTMERGEMAX_H_INCLUDED

#include "GridGame.h"  // Include GridGame.h for Position struct and constants
#include <vector>
#include <algorithm>
#include <cmath>
#include <utility> // For  pair
using namespace std;

/**
 * @class SmartMergeMax
 * @brief AI algorithm for a grid-based merge game that aims to create tiles with value 2.
 *        The algorithm evaluates moves by simulating them and scoring based on how close
 *        they get to creating the win value (2).
 */

class SmartMergeMax
{
private:
    // Use constants from GridGame.h instead of redefining them
    const int EMPTY=0;
    const int WIN_VALUE=2;
    const vector<char> DIRECTIONS = {'w', 's', 'a', 'd'};  // up, down, left, right
    const vector<char> PREFERENCE_ORDER = {'w', 'a', 's', 'd'};  // order for tie break

    // Create a copy of the grid for simulation
    vector<vector<int>> copyGrid(const vector<vector<int>>& original) const;

    // Simulate a move and count merges with weighting
    pair<vector<vector<int>>, float> simulateMoveAndCountMerges(
                              const vector<vector<int>>& grid,
                              const Position& pos,
                              char direction) const;

    // Modified from simulateProcessMovement to count merges with weights
    float simulateProcessMovementAndCountMerges(
                              Position& pos,
                              vector<vector<int>>& grid,
                              char dir) const;
public:
    SmartMergeMax(); // Constructor

    // Get the best move according to the merge maximization strategy
    char getBestMove(const vector<vector<int>>& grid, const Position& currentPos);
};

#endif // SMARTMERGEMAX_H_INCLUDED
