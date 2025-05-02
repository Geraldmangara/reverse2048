#include "SmartMergeMax.h"
#include "GridGame.h"
#include <iostream>
using namespace std;

SmartMergeMax::SmartMergeMax() {
    // Constructor is empty as constants are used from GridGame.h
}

// Create a copy of the grid for simulation
vector<vector<int>> SmartMergeMax::copyGrid(const vector<vector<int>>& original) const {
    return vector<vector<int>>(original);
}

// Simulate a move and count merges with weighting
pair<vector<vector<int>>, float> SmartMergeMax::simulateMoveAndCountMerges(
    const vector<vector<int>>& grid,
    const Position& pos,
    char direction) const {

    // Create a copy of the grid to work with
    vector<vector<int>> newGrid = copyGrid(grid);
    Position positionCopy = pos;

    // Simulate the move and count merges
    float mergeScore = simulateProcessMovementAndCountMerges(positionCopy, newGrid, direction);

    return {newGrid, mergeScore};
}

// Modified from simulateProcessMovement to count merges with weights
float SmartMergeMax::simulateProcessMovementAndCountMerges(
    Position& pos,
    vector<vector<int>>& grid,
    char dir) const {

    float mergeScore = 0;
    dir = tolower(dir);
    int gridSize = grid.size();

    struct MoveVector {
        int rowDelta;
        int colDelta;
        int startRow;
        int endRow;
        int rowStep;
        int startCol;
        int endCol;
        int colStep;
    };

    MoveVector moveVectors;

    switch(dir) {
        case 's': // Down
            moveVectors = {1, 0, gridSize-2, -1, -1, 0, gridSize, 1};
            break;
        case 'w': // Up
            moveVectors = {-1, 0, 1, gridSize, 1, 0, gridSize, 1};
            break;
        case 'a': // Left
            moveVectors = {0, -1, 0, gridSize, 1, 1, gridSize, 1};
            break;
        case 'd': // Right
            moveVectors = {0, 1, 0, gridSize, 1, gridSize-2, -1, -1};
            break;
        default:
            return 0; // No merges for invalid direction
    }

    for (int row = moveVectors.startRow; row != moveVectors.endRow; row += moveVectors.rowStep) {
        for (int col = moveVectors.startCol; col != moveVectors.endCol; col += moveVectors.colStep) {
            if (grid[row][col] != EMPTY) {
                int newRow = row;
                int newCol = col;

                while (true) {
                    int nextRow = newRow + moveVectors.rowDelta;
                    int nextCol = newCol + moveVectors.colDelta;

                    if (nextRow >= 0 && nextRow < gridSize &&
                        nextCol >= 0 && nextCol < gridSize &&
                        (grid[nextRow][nextCol] == EMPTY ||
                         grid[nextRow][nextCol] == grid[row][col])) {
                        newRow = nextRow;
                        newCol = nextCol;
                    } else {
                        break;
                    }
                }

                if (newRow != row || newCol != col) {
                    // If there's a merge (same values coming together)
                    if (grid[newRow][newCol] == grid[row][col]) {
                        int tileValue = grid[row][col];
                        int mergedValue = tileValue / 2; // The new value after merging
                        grid[newRow][newCol] = mergedValue;
                        grid[row][col] = EMPTY;

                        // Give extra points if this merge created a win value (2)
                        if (mergedValue == WIN_VALUE) {
                            // Massive bonus for creating a win condition
                            mergeScore += 10000;
                        } else {
                            // Otherwise, score based on how close we get to the win value
                            // The closer to 2, the higher the score
                            // Let's favor values that are closer to 2 in terms of merges needed
                            int mergesNeeded = log2(mergedValue) - log2(WIN_VALUE);
                            if (mergesNeeded > 0) {
                                // For values higher than 2, give points based on how close they are
                                mergeScore += 100.0 / mergesNeeded;
                            }
                        }
                    } else {
                        grid[newRow][newCol] = grid[row][col];
                        grid[row][col] = EMPTY;
                    }
                }
            }
        }
    }

    return mergeScore;
}

// Get the best move according to the merge maximization strategy
char SmartMergeMax::getBestMove(const vector<vector<int>>& grid, const Position& currentPos) {
    float maxMergeScore = -1;
    vector<char> bestMoves;

    // Try each possible direction
    for (char dir : DIRECTIONS) {
        // Simulate the move and count weighted merges
        auto [resultGrid, mergeScore] = simulateMoveAndCountMerges(grid, currentPos, dir);

        // If this move results in a grid change
        if (resultGrid != grid) {
            // If this move results in better merge score, it becomes our new best move
            if (mergeScore > maxMergeScore) {
                maxMergeScore = mergeScore;
                bestMoves.clear();
                bestMoves.push_back(dir);
            }
            // If it's tied with our current best, add it to the list of best moves
            else if (mergeScore == maxMergeScore) {
                bestMoves.push_back(dir);
            }
        }
    }

    // If we found valid moves
    if (!bestMoves.empty()) {
        // If we have multiple equally good moves, pick based on preference order
        if (bestMoves.size() > 1) {
            for (char prefDir : PREFERENCE_ORDER) {
                if (find(bestMoves.begin(), bestMoves.end(), prefDir) != bestMoves.end()) {
                    return prefDir;
                }
            }
        }
        return bestMoves[0];
    }

    // If no valid moves at all, return a default move
    return PREFERENCE_ORDER[0];
}
