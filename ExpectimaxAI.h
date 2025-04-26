

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




class ExpectimaxAI {
private:
    vector<vector<int>>& grid;
    Position& position;
    const int gridSize, maxDepth, EMPTY;
    int startNumber;
    vector<int> possibleSpawnValues;
    unordered_map<string, double> evalCache;
    // Decay parameters
    const double decayFactor = 0.7;  // Controls how quickly the weight decreases

    struct DirVector {
        int rowDelta, colDelta, startRow, endRow, rowStep, startCol, endCol, colStep;
    };
    unordered_map<char, DirVector> dirVectors;

    string gridToString(const vector<vector<int>>& g) const {
        string result;
        for (const auto& row : g)
            for (int val : row)
                result += to_string(val) + ",";
        return result;
    }

    bool isValidPosition(int row, int col) const {
        return row >= 0 && row < gridSize && col >= 0 && col < gridSize;
    }

    void initDirectionVectors() {
        dirVectors = {
            {'i', {-1, 0, 1, gridSize, 1, 0, gridSize, 1}},    // Up
            {'k', {1, 0, gridSize-2, -1, -1, 0, gridSize, 1}}, // Down
            {'j', {0, -1, 0, gridSize, 1, 1, gridSize, 1}},    // Left
            {'l', {0, 1, 0, gridSize, 1, gridSize-2, -1, -1}}  // Right
        };
    }

    // Initialize possible spawn values based on the starting number
    void initPossibleSpawnValues() {
        if (startNumber == 512) {
            possibleSpawnValues = {256, 128, 64};
        } else if (startNumber == 256) {
            possibleSpawnValues = {128, 64, 32};
        } else if (startNumber == 128) {
            possibleSpawnValues = {64, 32, 16};
        } else {
            // Default case - just use the startNumber
            possibleSpawnValues = {startNumber};
        }
    }

    // Calculate position weight based on exponential decay from a corner
    double getPositionWeight(int row, int col) const {
        // Using bottom-right corner as the preferred position
        int distanceFromCorner = (gridSize - 1 - row) + (gridSize - 1 - col);
        return pow(decayFactor, distanceFromCorner);
    }

    bool tryMove(const vector<vector<int>>& g, char dir) const {
        const auto& dv = dirVectors.at(dir);
        for (int i = dv.startRow; i != dv.endRow; i += dv.rowStep) {
            for (int j = dv.startCol; j != dv.endCol; j += dv.colStep) {
                if (g[i][j] != EMPTY) {
                    int ni = i + dv.rowDelta, nj = j + dv.colDelta;
                    if (isValidPosition(ni, nj) && (g[ni][nj] == EMPTY || g[ni][nj] == g[i][j]))
                        return true;
                }
            }
        }
        return false;
    }

    vector<vector<int>> simulateMove(const vector<vector<int>>& g, char dir) const {
        vector<vector<int>> newGrid = g;
        const auto& dv = dirVectors.at(dir);

        for (int i = dv.startRow; i != dv.endRow; i += dv.rowStep) {
            for (int j = dv.startCol; j != dv.endCol; j += dv.colStep) {
                if (newGrid[i][j] == EMPTY) continue;

                int ni = i, nj = j;
                while (true) {
                    int next_i = ni + dv.rowDelta, next_j = nj + dv.colDelta;
                    if (!isValidPosition(next_i, next_j)) break;

                    if (newGrid[next_i][next_j] == EMPTY) {
                        ni = next_i;
                        nj = next_j;
                    } else if (newGrid[next_i][next_j] == newGrid[i][j]) {
                        ni = next_i;
                        nj = next_j;
                        break;
                    } else {
                        break;
                    }
                }

                if (ni != i || nj != j) {
                    if (newGrid[ni][nj] == newGrid[i][j])
                        newGrid[ni][nj] /= 2;  // Merge by division
                    else
                        newGrid[ni][nj] = newGrid[i][j];
                    newGrid[i][j] = EMPTY;
                }
            }
        }
        return newGrid;
    }

    vector<Position> getEmptyCells(const vector<vector<int>>& g) const {
        vector<Position> emptyCells;
        for (int i = 0; i < gridSize; i++)
            for (int j = 0; j < gridSize; j++)
                if (g[i][j] == EMPTY)
                    emptyCells.push_back({i, j});
        return emptyCells;
    }

    bool hasValueOne(const vector<vector<int>>& g) const {
        for (int i = 0; i < gridSize; i++)
            for (int j = 0; j < gridSize; j++)
                if (g[i][j] == 1)
                    return true;
        return false;
    }

    bool checkGameOver(const vector<vector<int>>& g) const {
        if (hasValueOne(g)) return true;

        // Check for empty cells
        for (int i = 0; i < gridSize; i++)
            for (int j = 0; j < gridSize; j++)
                if (g[i][j] == EMPTY)
                    return false;

        // Check for possible merges
        for (int i = 0; i < gridSize; i++) {
            for (int j = 0; j < gridSize; j++) {
                if ((j < gridSize - 1 && g[i][j] == g[i][j+1]) ||
                    (i < gridSize - 1 && g[i][j] == g[i+1][j]))
                    return false;
            }
        }
        return true;
    }

    double evaluateGrid(const vector<vector<int>>& g) const {
        if (hasValueOne(g)) return DBL_MAX;

        double score = 0.0;
        int emptyCells = 0;
        double mergeOpportunities = 0.0;
        int minValue = INT_MAX;

        for (int i = 0; i < gridSize; i++) {
            for (int j = 0; j < gridSize; j++) {
                if (g[i][j] == EMPTY) {
                    emptyCells++;
                } else {
                    // Apply position weight with exponential decay
                    double positionWeight = getPositionWeight(i, j);

                    //SCORE: The most important line of code
                 score += (1000.0 / g[i][j]) * positionWeight;// Prefer smaller values with position-based weighting
                     score += log2(g[i][j]) * positionWeight;// second: the evaluation now gives a higher score for game states where larger-valued tiles (higher log2 value) are located in positions with higher position weight (closer to the bottom-right corner).
                     score -= (double)g[i][j] * positionWeight; // third:this gives a higher score for states where larger tiles are closer to the bottom-right corner. The increase in score is directly proportional to the tile's value and the position weight.
                      score -= ((double)g[i][j] * g[i][j]) * positionWeight;// fourth: This creates a stronger pull for the AI to get the absolute largest tiles into the bottom-right corner. Might be useful if consolidating the very first high-value tiles is the main bottleneck to reaching '1'.
                    // Track minimum value
                    if (g[i][j] < minValue)
                        minValue = g[i][j];

                    // Count merge opportunities
                    if (j < gridSize - 1 && g[i][j] == g[i][j+1])
                        mergeOpportunities += 1.0;
                    if (i < gridSize - 1 && g[i][j] == g[i+1][j])
                        mergeOpportunities += 1.0;
                }
            }
        }

        return score + (4* emptyCells) + (10.0 * mergeOpportunities);
    }

    double expectimax(const vector<vector<int>>& g, int depth, bool isMaxPlayer) {
        // Check cache
        string cacheKey = gridToString(g) + to_string(depth) + (isMaxPlayer ? "m" : "c");
        if (evalCache.find(cacheKey) != evalCache.end())
            return evalCache[cacheKey];

        // Terminal conditions
        if (hasValueOne(g)) return DBL_MAX;
        if (depth == 0 || checkGameOver(g))
            return evaluateGrid(g);

        double result;
        if (isMaxPlayer) {
            result = -DBL_MAX;
            for (char dir : {'i', 'j', 'k', 'l'}) {
                if (tryMove(g, dir)) {
                    auto newGrid = simulateMove(g, dir);
                    result = max(result, expectimax(newGrid, depth - 1, false));
                }
            }
            if (result == -DBL_MAX)
                result = evaluateGrid(g);
        } else {
            // Chance node - now considering multiple possible spawn values
            auto emptyCells = getEmptyCells(g);
            if (emptyCells.empty())
                return expectimax(g, depth - 1, true);

            result = 0.0;
            double cellProb = 1.0 / emptyCells.size();
            double valueProb = 1.0 / possibleSpawnValues.size();

            // For each empty cell and each possible value
            for (const auto& pos : emptyCells) {
                for (int value : possibleSpawnValues) {
                    auto newGrid = g;
                    newGrid[pos.row][pos.col] = value;
                    result += cellProb * valueProb * expectimax(newGrid, depth - 1, true);
                }
            }
        }

        evalCache[cacheKey] = result;
        return result;
    }

public:
    ExpectimaxAI(vector<vector<int>>& g, Position& pos, int size,
                 int initialNumber, int depth, int empty)
        : grid(g), position(pos), gridSize(size), maxDepth(depth),
          EMPTY(empty), startNumber(initialNumber)
    {
        initDirectionVectors();
        initPossibleSpawnValues();
    }

    // Added method to customize decay factor
    void setDecayFactor(double factor) {
        const_cast<double&>(decayFactor) = factor;
    }

    // Update spawn values when the game configuration changes
    void updateSpawnValues(int newStartNumber) {
        startNumber = newStartNumber;
        initPossibleSpawnValues();
    }

    char getBestMove() {
        char bestMove = 'n';
        double bestScore = -DBL_MAX;

        for (char dir : {'i', 'j', 'k', 'l'}) {
            if (!tryMove(grid, dir)) continue;
            auto newGrid = simulateMove(grid, dir);
            double score = expectimax(newGrid, maxDepth - 1, false);
            if (score > bestScore) {
                bestScore = score;
                bestMove = dir;
            }
        }
        return bestMove;
    }

    void resetCache() { evalCache.clear(); }

    bool playOneStep(GridGame* game);
};


#endif // EXPECTIMAXAI_H_INCLUDED
