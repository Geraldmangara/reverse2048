#include "ExpectimaxAI.h"
#include "GridGame.h"
using namespace std;
// Initialize direction vectors for movement
void ExpectimaxAI::initDirectionVectors()
{
    dirVectors =
    {
        {'i', {-1, 0, 1, gridSize, 1, 0, gridSize, 1}},    // Up
        {'k', {1, 0, gridSize-2, -1, -1, 0, gridSize, 1}}, // Down
        {'j', {0, -1, 0, gridSize, 1, 1, gridSize, 1}},    // Left
        {'l', {0, 1, 0, gridSize, 1, gridSize-2, -1, -1}}  // Right
    };
}

// Initialize possible spawn values based on the starting number
void ExpectimaxAI::initPossibleSpawnValues()
{
    if (startNumber == 512)
    {
        possibleSpawnValues = {256, 128, 64};
    }
    else if (startNumber == 256)
    {
        possibleSpawnValues = {128, 64, 32};
    }
    else if (startNumber == 128)
    {
        possibleSpawnValues = {64, 32, 16};
    }
    else
    {
        // Default case - just use the startNumber
        possibleSpawnValues = {startNumber};
    }
}

// Calculate position weight based on exponential decay from a corner
double ExpectimaxAI::getPositionWeight(int row, int col) const
{
    // Using bottom-right corner as the preferred position
    int distanceFromCorner = (gridSize - 1 - row) + (gridSize - 1 - col);
    return pow(decayFactor, distanceFromCorner);
}

// Convert grid to string for caching
string ExpectimaxAI::gridToString(const vector<vector<int>>& g) const
{
    string result;
    for (const auto& row : g)
        for (int val : row)
            result += to_string(val) + ",";
    return result;
}

// Check if position is within grid bounds
bool ExpectimaxAI::isValidPosition(int row, int col) const
{
    return row >= 0 && row < gridSize && col >= 0 && col < gridSize;
}

// Check if a move in the given direction is possible
bool ExpectimaxAI::tryMove(const vector<vector<int>>& g, char dir) const
{
    const auto& dv = dirVectors.at(dir);
    for (int i = dv.startRow; i != dv.endRow; i += dv.rowStep)
    {
        for (int j = dv.startCol; j != dv.endCol; j += dv.colStep)
        {
            if (g[i][j] != EMPTY)
            {
                int ni = i + dv.rowDelta, nj = j + dv.colDelta;
                if (isValidPosition(ni, nj) && (g[ni][nj] == EMPTY || g[ni][nj] == g[i][j]))
                    return true;
            }
        }
    }
    return false;
}

// Simulate a move in the given direction
vector<vector<int>> ExpectimaxAI::simulateMove(const vector<vector<int>>& g, char dir) const
{
    vector<vector<int>> newGrid = g;
    const auto& dv = dirVectors.at(dir);

    for (int i = dv.startRow; i != dv.endRow; i += dv.rowStep)
    {
        for (int j = dv.startCol; j != dv.endCol; j += dv.colStep)
        {
            if (newGrid[i][j] == EMPTY) continue;

            int ni = i, nj = j;
            while (true)
            {
                int next_i = ni + dv.rowDelta, next_j = nj + dv.colDelta;
                if (!isValidPosition(next_i, next_j)) break;

                if (newGrid[next_i][next_j] == EMPTY)
                {
                    ni = next_i;
                    nj = next_j;
                }
                else if (newGrid[next_i][next_j] == newGrid[i][j])
                {
                    ni = next_i;
                    nj = next_j;
                    break;
                }
                else
                {
                    break;
                }
            }

            if (ni != i || nj != j)
            {
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

// Get all empty cells in the grid
vector<Position> ExpectimaxAI::getEmptyCells(const vector<vector<int>>& g) const
{
    vector<Position> emptyCells;
    for (int i = 0; i < gridSize; i++)
        for (int j = 0; j < gridSize; j++)
            if (g[i][j] == EMPTY)
                emptyCells.push_back({i, j});
    return emptyCells;
}

// Check if the grid has a value of 1 (win condition)
bool ExpectimaxAI::hasValueOne(const vector<vector<int>>& g) const
{
    for (int i = 0; i < gridSize; i++)
        for (int j = 0; j < gridSize; j++)
            if (g[i][j] == 1)
                return true;
    return false;
}

// Check if the game is over (no valid moves)
bool ExpectimaxAI::checkGameOver(const vector<vector<int>>& g) const
{
    if (hasValueOne(g)) return true;

    // Check for empty cells
    for (int i = 0; i < gridSize; i++)
        for (int j = 0; j < gridSize; j++)
            if (g[i][j] == EMPTY)
                return false;

    // Check for possible merges
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            if ((j < gridSize - 1 && g[i][j] == g[i][j+1]) ||
                    (i < gridSize - 1 && g[i][j] == g[i+1][j]))
                return false;
        }
    }
    return true;
}

// Evaluate the grid state
double ExpectimaxAI::evaluateGrid(const vector<vector<int>>& g) const
{
    if (hasValueOne(g)) return DBL_MAX;

    double score = 0.0;
    int emptyCells = 0;
    double mergeOpportunities = 0.0;
    int minValue = INT_MAX;

    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            if (g[i][j] == EMPTY)
            {
                emptyCells++;
            }
            else
            {
                // Apply position weight with exponential decay
                double positionWeight = getPositionWeight(i, j);

                //SCORE: The most important line of code
                score += (1000.0 / g[i][j]) * positionWeight; // Prefer smaller values with position-based weighting

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

    return score + (4 * emptyCells) + (10.0 * mergeOpportunities);
}

// Expectimax algorithm implementation
double ExpectimaxAI::expectimax(const vector<vector<int>>& g, int depth, bool isMaxPlayer)
{
    // Check cache
    string cacheKey = gridToString(g) + to_string(depth) + (isMaxPlayer ? "m" : "c");
    if (evalCache.find(cacheKey) != evalCache.end())
        return evalCache[cacheKey];

    // Terminal conditions
    if (hasValueOne(g)) return DBL_MAX;
    if (depth == 0 || checkGameOver(g))
        return evaluateGrid(g);

    double result;
    if (isMaxPlayer)
    {
        result = -DBL_MAX;
        for (char dir :
                {'i', 'j', 'k', 'l'
                })
        {
            if (tryMove(g, dir))
            {
                auto newGrid = simulateMove(g, dir);
                result = max(result, expectimax(newGrid, depth - 1, false));
            }
        }
        if (result == -DBL_MAX)
            result = evaluateGrid(g);
    }
    else
    {
        // Chance node - now considering multiple possible spawn values
        auto emptyCells = getEmptyCells(g);
        if (emptyCells.empty())
            return expectimax(g, depth - 1, true);

        result = 0.0;
        double cellProb = 1.0 / emptyCells.size();
        double valueProb = 1.0 / possibleSpawnValues.size();

        // For each empty cell and each possible value
        for (const auto& pos : emptyCells)
        {
            for (int value : possibleSpawnValues)
            {
                auto newGrid = g;
                newGrid[pos.row][pos.col] = value;
                result += cellProb * valueProb * expectimax(newGrid, depth - 1, true);
            }
        }
    }

    evalCache[cacheKey] = result;
    return result;
}

// Constructor
ExpectimaxAI::ExpectimaxAI(vector<vector<int>>& g, Position& pos, int size,
                           int initialNumber, int depth, int empty)
    : grid(g), position(pos), gridSize(size), maxDepth(depth),
      EMPTY(empty), startNumber(initialNumber)
{
    initDirectionVectors();
    initPossibleSpawnValues();
}

// Set decay factor
void ExpectimaxAI::setDecayFactor(double factor)
{
    const_cast<double&>(decayFactor) = factor;
}

// Update spawn values
void ExpectimaxAI::updateSpawnValues(int newStartNumber)
{
    startNumber = newStartNumber;
    initPossibleSpawnValues();
}

// Get best move
char ExpectimaxAI::getBestMove()
{
    char bestMove = 'n';
    double bestScore = -DBL_MAX;

    for (char dir :
            {'i', 'j', 'k', 'l'
            })
    {
        if (!tryMove(grid, dir)) continue;
        auto newGrid = simulateMove(grid, dir);
        double score = expectimax(newGrid, maxDepth - 1, false);
        if (score > bestScore)
        {
            bestScore = score;
            bestMove = dir;
        }
    }
    return bestMove;
}

// Reset cache
void ExpectimaxAI::resetCache()
{
    evalCache.clear();
}

// Play one step
bool ExpectimaxAI::playOneStep(GridGame* game)
{
    char bestMove = getBestMove();
    if (bestMove != 'n')
    {
        return game->performProcessMovement(position, grid, bestMove);
    }
    return false;
}
