#include "GridGame.h"
#include "ExpectimaxAI.h"
#include "SmartMergeMax.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <thread>
#include <chrono>
#include <utility>
#include <cmath>
using namespace std;

// Define constants used within GridGame and potentially by AIs
const int GridGame::EMPTY = 0;
const int GridGame::WIN_VALUE = 2; // In reverse 2048, we want to reach 2
const int GridGame::MIN_GRID_SIZE = 3;
const int GridGame::MAX_GRID_SIZE = 5;
const int GridGame::MAX_MOVES = 1000; // Move limit per AI
// Initialize VALID_NUMBERS
const std::vector<int> GridGame::VALID_NUMBERS = {
    2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
};


// Initialize possible spawn values based on the starting number
void GridGame::initPossibleSpawnValues()
{
    possibleSpawnValues.clear();
    if (currentNumber == 512)
    {
        possibleSpawnValues = {256, 128, 64};
    }
    else if (currentNumber == 256)
    {
        possibleSpawnValues = {128, 64, 32};
    }
    else if (currentNumber == 128)
    {
        possibleSpawnValues = {64, 32, 16};
    }
    else
    {
        // Default case - just use the currentNumber
        possibleSpawnValues = {currentNumber};
    }
}

// Checks if config file values are acceptable
void GridGame::validateConfiguration() const
{
    if (gridSize < MIN_GRID_SIZE || gridSize > MAX_GRID_SIZE)
    {
        throw invalid_argument("Grid size must be between 3 and 5");
    }
    if (find(VALID_NUMBERS.begin(), VALID_NUMBERS.end(), currentNumber) == VALID_NUMBERS.end())
    {
        throw invalid_argument("Number must be 128, 256, or 512");
    }
}

void GridGame::initializeGrids()
{
    grid1 = vector<vector<int>>(gridSize, vector<int>(gridSize, EMPTY));
    grid2 = vector<vector<int>>(gridSize, vector<int>(gridSize, EMPTY));

    // Initialize possible spawn values
    initPossibleSpawnValues();

    // Seed the random number generator with current time for better randomness
    auto timeNow = chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(random_device()() ^ static_cast<uint32_t>(timeNow));

    // Create a list of all possible positions
    vector<Position> allPositions;
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            allPositions.push_back({i, j});
        }
    }

    // Shuffle all positions thoroughly
    shuffle(allPositions.begin(), allPositions.end(), rng);

    // Use the first two shuffled positions for initial tiles
    Position pos1 = allPositions[0];
    Position pos2 = allPositions[1];

    // Place tiles at these positions
    grid1[pos1.row][pos1.col] = currentNumber;
    grid1[pos2.row][pos2.col] = currentNumber;
    grid2[pos1.row][pos1.col] = currentNumber;
    grid2[pos2.row][pos2.col] = currentNumber;
}

// Figures out where the cursor would move given a direction
Position GridGame::calculateNewPosition(Position pos, char dir) const
{
    dir = tolower(dir);
    Position newPos = pos;
    switch (dir)
    {
    case 'w':
    case 'i':
        newPos.row = max(0, newPos.row - 1); // Prevent going out of bounds
        break;
    case 's':
    case 'k':
        newPos.row = min(gridSize - 1, newPos.row + 1); // Prevent going out of bounds
        break;
    case 'a':
    case 'j':
        newPos.col = max(0, newPos.col - 1); // Prevent going out of bounds
        break;
    case 'd':
    case 'l':
        newPos.col = min(gridSize - 1, newPos.col + 1); // Prevent going out of bounds
        break;
    }
    return newPos;
}

// Checks if a given grid position is within bounds
bool GridGame::isValidPosition(Position pos) const
{
    return pos.row >= 0 && pos.row < gridSize &&
           pos.col >= 0 && pos.col < gridSize;
}

// Adds a new tile with a random value from possibleSpawnValues in an empty spot
void GridGame::spawnRandomNumber(vector<vector<int>>& grid)
{
    vector<Position> emptyCells;
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            if (grid[i][j] == EMPTY)
            {
                emptyCells.push_back({i, j});
            }
        }
    }

    if (!emptyCells.empty())
    {
        uniform_int_distribution<int> cellDist(0, emptyCells.size() - 1);
        Position spawnPos = emptyCells[cellDist(rng)];

        // Choose a random value from possibleSpawnValues
        uniform_int_distribution<int> valueDist(0, possibleSpawnValues.size() - 1);
        int valueIndex = valueDist(rng);

        grid[spawnPos.row][spawnPos.col] = possibleSpawnValues[valueIndex];
    }
}

// Moves tiles in a given direction and handles merging
bool GridGame::processMovement(Position& pos, vector<vector<int>>& grid, char dir)
{
    bool gridChanged = false;
    dir = tolower(dir);

    struct MoveVector
    {
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

    switch(dir)
    {
    case 's':
    case 'k': // Down
        moveVectors = {1, 0, gridSize-2, -1, -1, 0, gridSize, 1};
        break;
    case 'w':
    case 'i': // Up
        moveVectors = {-1, 0, 1, gridSize, 1, 0, gridSize, 1};
        break;
    case 'a':
    case 'j': // Left
        moveVectors = {0, -1, 0, gridSize, 1, 1, gridSize, 1};
        break;
    case 'd':
    case 'l': // Right
        moveVectors = {0, 1, 0, gridSize, 1, gridSize-2, -1, -1};
        break;
    default:
        return false;
    }

    for (int row = moveVectors.startRow; row != moveVectors.endRow; row += moveVectors.rowStep)
    {
        for (int col = moveVectors.startCol; col != moveVectors.endCol; col += moveVectors.colStep)
        {
            if (grid[row][col] != EMPTY)
            {
                int newRow = row;
                int newCol = col;

                while (true)
                {
                    int nextRow = newRow + moveVectors.rowDelta;
                    int nextCol = newCol + moveVectors.colDelta;

                    if (nextRow >= 0 && nextRow < gridSize &&
                            nextCol >= 0 && nextCol < gridSize &&
                            (grid[nextRow][nextCol] == EMPTY ||
                             grid[nextRow][nextCol] == grid[row][col]))
                    {
                        newRow = nextRow;
                        newCol = nextCol;
                    }
                    else
                    {
                        break;
                    }
                }

                if (newRow != row || newCol != col)
                {
                    if (grid[newRow][newCol] == grid[row][col])
                    {
                        // For reverse 2048, we divide by 2 to get smaller numbers
                        grid[newRow][newCol] /= 2;
                        grid[row][col] = EMPTY;
                    }
                    else
                    {
                        grid[newRow][newCol] = grid[row][col];
                        grid[row][col] = EMPTY;
                    }
                    gridChanged = true;
                }
            }
        }
    }

    if (gridChanged)
    {
        Position newPos = calculateNewPosition(pos, dir);
        if (isValidPosition(newPos))
        {
            pos = newPos;
        }
        spawnRandomNumber(grid);
        return true;
    }

    return false;
}

// Checks if the game has reached a win or stalemate for a grid
bool GridGame::checkGameOver(const vector<vector<int>>& grid) const
{
    // Check for win condition (tile with value 2)
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            if (grid[i][j] == WIN_VALUE)
            {
                return true; // Win condition
            }
        }
    }

    // Check if grid is full
    bool gridFull = true;
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            if (grid[i][j] == EMPTY)
            {
                gridFull = false;
                break;
            }
        }
        if (!gridFull) break;
    }

    if (gridFull)
    {
        // Check if any adjacent tiles can be merged
        for (int i = 0; i < gridSize; i++)
        {
            for (int j = 0; j < gridSize - 1; j++)
            {
                if (grid[i][j] == grid[i][j+1])
                {
                    return false; // Merge is possible, game not over
                }
            }
        }

        for (int j = 0; j < gridSize; j++)
        {
            for (int i = 0; i < gridSize - 1; i++)
            {
                if (grid[i][j] == grid[i+1][j])
                {
                    return false; // Merge is possible, game not over
                }
            }
        }

        return true; // Grid is full and no merges possible
    }

    return false; // Grid not full, game not over
}

// Constructor that loads config and starts game
GridGame::GridGame(int currentNumber, int gridSize) :
    currentNumber(currentNumber),
    gridSize(gridSize),
    rng(random_device()()),
    grid1GameOver(false),
    grid2GameOver(false),
    lastMoveGrid1(' '),
    lastMoveGrid2(' '),
    isFirstDisplay(true),
    grid1Reason(GameOverReason::NOT_OVER),
    grid2Reason(GameOverReason::NOT_OVER),
    Ai1Count(0),
    Ai2Count(0)
{
    validateConfiguration();
    initializeGrids();

    // Initialize game state tracking variables
    pos1 = {0, 0};
    pos2 = {0, 0};

    // Initialize AIs
    ai1 = new SmartMergeMax();
    ai2 = new ExpectimaxAI(grid2, pos2, gridSize, currentNumber, 4, EMPTY);
}

// Destructor to clean up the AIs
GridGame::~GridGame()
{
    if (ai1) delete ai1;
    if (ai2) delete ai2;
}

void GridGame::handleInput(char input)
{
    input = tolower(input);
    bool gridChanged = false;

    if (input == 'q') return;

    // Manual control
    if (input == 'w' || input == 'a' || input == 's' || input == 'd')
    {
        if (!grid1GameOver)
        {
            gridChanged = processMovement(pos1, grid1, input);
            lastMoveGrid1 = input;

            if (gridChanged)
            {
                if (checkGameOver(grid1))
                {
                    grid1GameOver = true;
                    displayGameState();
                    if (hasWon(grid1))
                    {
                        cout << "\nGrid 1 won by reaching " << WIN_VALUE << "!\n";
                        return;
                    }
                    cout << "\nGrid 1 has no more moves possible.\n";
                }
            }
        }
    }
    else if (input == 'i' || input == 'j' || input == 'k' || input == 'l')
    {
        if (!grid2GameOver)
        {
            gridChanged = processMovement(pos2, grid2, input);
            lastMoveGrid2 = input;

            if (gridChanged)
            {
                if (checkGameOver(grid2))
                {
                    grid2GameOver = true;
                    displayGameState();
                    if (hasWon(grid2))
                    {
                        cout << "\nGrid 2 won by reaching " << WIN_VALUE << "!\n";
                        return;
                    }
                    cout << "\nGrid 2 has no more moves possible.\n";
                }
            }
        }
    }
    else if (input == 'a')      // AI play one step
    {
        aiPlayOneStep();
    }
    else if (input == 'p')      // AI play until game over
    {
        aiPlayUntilGameOver(); // This will now return the result
    }
    else if (input == '1')      // AI1 play one step
    {
        ai1PlayOneStep();
    }
    else if (input == '2')      // AI2 play one step
    {
        ai2PlayOneStep();
    }

    if (gridChanged)
    {
        isFirstDisplay = false;
        displayGameState();
    }
    else if (input != 'q' && input != 'a' && input != 'p' && input != '1' && input != '2')
    {
        cout << "Invalid move!\n";
    }
}

// Convert direction char to readable string
string GridGame::directionToString(char dir) const
{
    switch(tolower(dir))
    {
    case 'w':
    case 'i':
        return "U";
    case 's':
    case 'k':
        return "D";
    case 'a':
    case 'j':
        return "L";
    case 'd':
    case 'l':
        return "R";
    default:
        return "";
    }
}

// Prints out both grids side-by-side with current number
void GridGame::displayGameState()
{
    // Header
    cout << "---------------------------------------------------------------------\n";
    if (isFirstDisplay)
    {
        cout << "reverse " << currentNumber << " (" << gridSize << " x " << gridSize << ") - Initial Board\n";
    }
    else
    {
        cout << "reverse " << currentNumber << " (" << gridSize << " x " << gridSize << ")\n";
        if (lastMoveGrid1 != ' ')
        {
            cout << "Grid 1 (AI1) Move: " << directionToString(lastMoveGrid1) << "\n";
        }
        if (lastMoveGrid2 != ' ')
        {
            cout << "Grid 2 (AI2) Move: " << directionToString(lastMoveGrid2) << "\n";
        }
    }
    cout << "---------------------------------------------------------------------\n";

    // Display both grids side by side
    for (int row = 0; row < gridSize; ++row)
    {
        // Grid 1 (SmartMergeMax AI)
        cout << "|";
        for (int col = 0; col < gridSize; ++col)
        {
            if (grid1[row][col] == EMPTY)
                cout << setw(5) << " - ";
            else
                cout << setw(5) << grid1[row][col];
        }
        cout << " |"; // Close Grid 1

        cout << "     "; // Space between the two grids

        // Grid 2 (ExpectimaxAI)
        cout << "|";
        for (int col = 0; col < gridSize; ++col)
        {
            if (grid2[row][col] == EMPTY)
                cout << setw(5) << " - ";
            else
                cout << setw(5) << grid2[row][col];
        }
        cout << " |"; // Close Grid 2

        cout << "\n";
    }

    // Bottom border
    cout << "---------------------------------------------------------------------\n";
}

// Shows movement keys to the player
void GridGame::showControls() const
{
    cout << "\nControls:\n"
         << "WASD - Manual control of Grid 1\n"
         << "IJKL - Manual control of Grid 2\n"
         << "1 - Let AI1 (SmartMergeMax) play one step on Grid 1\n"
         << "2 - Let AI2 (Expectimax) play one step on Grid 2\n"
         << "A - Let both AIs play one step\n"
         << "P - Let both AIs play until game over\n"
         << "Q - Quit\n\n";
}

// Checks if one of the algorithm has won
bool GridGame::hasWon(const vector<vector<int>>& grid) const
{
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            if (grid[i][j] == WIN_VALUE)
            {
                return true;
            }
        }
    }
    return false;
}

// Let the AI for grid 1 play one step
bool GridGame::ai1PlayOneStep()
{
    if (grid1GameOver)
    {
        return false;
    }

    // Get best move from SmartMergeMax AI
    char bestMove = ai1->getBestMove(grid1, pos1);
    lastMoveGrid1 = bestMove;

    // Execute the move
    bool validMove = processMovement(pos1, grid1, bestMove);
    if (validMove)
    {

        Ai1Count++;
        isFirstDisplay = false;

        if (checkGameOver(grid1))
        {
            grid1GameOver = true;
            // Check specifically for win condition
            if (hasWon(grid1)) {
                grid1Reason = GameOverReason::WIN;
            } else {
                grid1Reason = GameOverReason::STALEMATE_BOARD_FULL; // Assuming board full stalemate if not a win
            }
        }
    }
    else
    {
        // If no valid move, game is over for this grid
        grid1GameOver = true;
        grid1Reason = GameOverReason::STALEMATE_NO_MOVES; // Use specific reason
    }

    return validMove;
}

// Let the AI for grid 2 play one step
bool GridGame::ai2PlayOneStep()
{
    if (grid2GameOver)
    {
        return false;
    }

    // Reset cache for AI2 (ExpectimaxAI)
    ai2->resetCache();

    // Let AI2 play one step
    bool validMove = ai2->playOneStep(this);

    if (validMove)
    {
        Ai2Count++;
        isFirstDisplay = false;

        if (checkGameOver(grid2))
        {
            grid2GameOver = true;
            // Check specifically for win condition
            if (hasWon(grid2)) {
                grid2Reason = GameOverReason::WIN;
            } else {
                grid2Reason = GameOverReason::STALEMATE_BOARD_FULL; // Assuming board full stalemate if not a win
            }
        }
    }
    else
    {
        // If no valid move, game is over for this grid
        grid2GameOver = true;
        grid2Reason = GameOverReason::STALEMATE_NO_MOVES; // Use specific reason
    }

    return validMove;
}

// Let both AIs play one step
bool GridGame::aiPlayOneStep()
{
    bool gridChanged = false;

    // Let AI1 play on Grid 1
    if (!grid1GameOver)
    {
        bool grid1Changed = ai1PlayOneStep(); // ai1PlayOneStep handles its own game over check
        gridChanged = gridChanged || grid1Changed;
    }

    // Let AI2 play on Grid 2
    if (!grid2GameOver)
    {
        ai2->resetCache(); // Reset cache before AI2's turn
        bool grid2Changed = ai2PlayOneStep(); // ai2PlayOneStep handles its own game over check
        gridChanged = gridChanged || grid2Changed;
    }

    // After both AIs have attempted a move, check for opponent winning
    if (grid1Reason == GameOverReason::WIN && !grid2GameOver) {
        grid2GameOver = true;
        grid2Reason = GameOverReason::OPPONENT_WON;
    }
    if (grid2Reason == GameOverReason::WIN && !grid1GameOver) {
        grid1GameOver = true;
        grid1Reason = GameOverReason::OPPONENT_WON;
    }

    return gridChanged; // Return true if at least one grid changed
}

// Plays the game automatically using AIs until one finishes or limit reached.
// Returns a GameResult structure containing the outcome details and history.
GameResult GridGame::aiPlayUntilGameOver()
{
    // Clear history for the new game
    gameHistory.clear();

    // Store the initial state
    gameHistory.push_back({grid1, grid2, ' ', ' ', 0}); // Step 0 for initial state

    int moveNumber = 0;
    while (!grid1GameOver || !grid2GameOver)
    {


        // Play one step for both AIs (if not already over)
        bool movedThisTurn = aiPlayOneStep();

        // Increment move number and store history if any grid changed
        if (movedThisTurn) {
      char move1Before = lastMoveGrid1;
        char move2Before = lastMoveGrid2;
            moveNumber++;
            // Store the state after this move
            gameHistory.push_back({grid1, grid2, move1Before, move2Before, moveNumber});
        }

        // Check for move limit after each step
        if (!grid1GameOver && Ai1Count >= MAX_MOVES) {
            grid1GameOver = true;
            grid1Reason = GameOverReason::MOVE_LIMIT_REACHED;
        }
        if (!grid2GameOver && Ai2Count >= MAX_MOVES) {
            grid2GameOver = true;
            grid2Reason = GameOverReason::MOVE_LIMIT_REACHED;
        }

        // Check if both games are now over
        if (grid1GameOver && grid2GameOver) {
            break; // Exit loop if both games finished
        }
    }

    // Game is over, populate and return the GameResult
    return GameResult(currentNumber, gridSize, grid1Reason, grid2Reason, Ai1Count, Ai2Count, gameHistory);
}

// Starts the main game loop
void GridGame::run()
{
    showControls();
    displayGameState();

    char input;
    do
    {
        cout << "> ";
        cin >> input;

        input = tolower(input);
        handleInput(input);

    }
    while (tolower(input) != 'q');
}

// Getter method for processMovement to be accessible by the AIs
bool GridGame::performProcessMovement(Position& pos, vector<vector<int>>& grid, char dir)
{
    if (&grid == &grid1) {
        lastMoveGrid1 = dir;
    } else if (&grid == &grid2) {
        lastMoveGrid2 = dir;
    }

    return processMovement(pos, grid, dir);
}

// Get string description of game over reason
string GridGame::getReasonString(GameOverReason reason, int moveCount) const {
    switch (reason) {
        case GameOverReason::NOT_OVER:
            return "Not Over";
        case GameOverReason::WIN:
            return "Win";
        case GameOverReason::STALEMATE_BOARD_FULL:
            return "Board Full";
        case GameOverReason::STALEMATE_NO_MOVES:
            return "No Valid Moves";
        case GameOverReason::MOVE_LIMIT_REACHED:
             return "Move Limit Reached (" + to_string(moveCount) + " moves)";
        case GameOverReason::OPPONENT_WON:
             return "Opponent Won";
        default:
            return "Unknown";
    }
}

GameOverReason GridGame::checkGameOverReason(const vector<vector<int>>& grid) const {
    // Check for win condition (tile with value 2)
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j] == WIN_VALUE) {
                return GameOverReason::WIN;
            }
        }
    }

    // Check for any empty cells
    bool hasEmpty = false;
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j] == EMPTY) {
                hasEmpty = true;
                break;
            }
        }
        if (hasEmpty) break;
    }

    // If there are empty cells, check if moves are possible
    if (hasEmpty) {
        return GameOverReason::NOT_OVER; // If there are empty cells, moves are still possible
    } else {
        // Board is full, check for possible merges
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize - 1; ++j) {
                if (grid[i][j] == grid[i][j+1]) {
                    return GameOverReason::NOT_OVER; // Horizontal merge possible
                }
            }
        }

        for (int j = 0; j < gridSize; ++j) {
            for (int i = 0; i < gridSize - 1; ++i) {
                if (grid[i][j] == grid[i+1][j]) {
                    return GameOverReason::NOT_OVER; // Vertical merge possible
                }
            }
        }

        return GameOverReason::STALEMATE_BOARD_FULL;
    }
}
