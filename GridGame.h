#ifndef GRIDGAME_H
#define GRIDGAME_H

#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <thread>
#include <chrono>
#include <utility>
#include <cmath>

// Use standard namespace for convenience in this project
using namespace std;

// Forward declarations for AI classes
class SmartMergeMax;
class ExpectimaxAI;

// Position structure to represent grid coordinates
struct Position {
    int row; // Row index
    int col; // Column index

    // Default constructor
    Position() : row(0), col(0) {}
    // Parameterized constructor
    Position(int r, int c) : row(r), col(c) {}

    // Equality operator for comparing positions
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    // Inequality operator (useful for completeness)
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

// Enum to specify the reason for a game over state
enum GameOverReason {
    NOT_OVER,             // Game is still in progress
    WIN,                  // Reached the target value (2)
    STALEMATE_BOARD_FULL, // Board is full, and no more merges are possible
    STALEMATE_NO_MOVES,   // Board is not full, but no valid moves or merges are possible
    MOVE_LIMIT_REACHED,    // Maximum allowed moves reached
    OPPONENT_WON          // The other AI won first
};

// Structure to hold the state at each step for history tracking
struct GameStep {
    vector<vector<int>> grid1State; // State of AI1's grid at this step
    vector<vector<int>> grid2State; // State of AI2's grid at this step
    char move1;                     // Move made by AI1 *leading to* this state (' ' for initial state)
    char move2;                     // Move made by AI2 *leading to* this state (' ' for initial state)
    int stepNumber;                 // The step number (0 for initial state, 1 for first move, etc.)

    // Default constructor
    GameStep() : move1(' '), move2(' '), stepNumber(0) {}
    // Parameterized constructor
    GameStep(const vector<vector<int>>& g1, const vector<vector<int>>& g2, char m1, char m2, int step)
        : grid1State(g1), grid2State(g2), move1(m1), move2(m2), stepNumber(step) {}
};

// Structure to hold the result of a game
struct GameResult {
    int startNumber;        // The starting number for the game
    int gridSize;           // The size of the grid
    GameOverReason grid1Reason; // Reason for AI1's game over
    GameOverReason grid2Reason; // Reason for AI2's game over
    int ai1MoveCount;       // Total moves made by AI1
    int ai2MoveCount;       // Total moves made by AI2
    vector<GameStep> history; // History of game states and moves

    // Default constructor
    GameResult() : startNumber(0), gridSize(0), grid1Reason(GameOverReason::NOT_OVER),
                   grid2Reason(GameOverReason::NOT_OVER), ai1MoveCount(0), ai2MoveCount(0) {}

    // Parameterized constructor
    GameResult(int startNum, int size, GameOverReason reason1, GameOverReason reason2, int count1, int count2, const vector<GameStep>& hist)
        : startNumber(startNum), gridSize(size), grid1Reason(reason1), grid2Reason(reason2),
          ai1MoveCount(count1), ai2MoveCount(count2), history(hist) {}
};


// The main class representing the grid game and managing the AIs
class GridGame {
public:
     static const int EMPTY;
    static const int WIN_VALUE;
    static const int MIN_GRID_SIZE;
    static const int MAX_GRID_SIZE;
    static const int MAX_MOVES;
    static const  vector<int> VALID_NUMBERS;
    // --- Constructor and Destructor ---

    // Constructor: Initializes the game with a starting number and grid size.
    GridGame(int startNumber, int gridSize);

    // Destructor: Cleans up dynamically allocated resources (AI objects).
    ~GridGame();

    // --- Game Flow Control ---

    // Runs the game loop (primarily for human interaction, but kept).
    // In this AI-only project, aiPlayUntilGameOver is the main entry point.
    void run();

    // Plays the game automatically using AIs until one finishes or limit reached.
    // Returns a GameResult structure containing the outcome details and history.
    GameResult aiPlayUntilGameOver(); // Modified to return GameResult

    // --- AI Interaction Methods ---

    // Lets AI1 play one step on its grid. Returns true if a move was made.
    bool ai1PlayOneStep();

    // Lets AI2 play one step on its grid. Returns true if a move was made.
    bool ai2PlayOneStep();

    // Lets both AIs play one step if their respective games are not over.
    bool aiPlayOneStep();

    // Public wrapper for the private processMovement, allowing AI classes to call it.
    // Processes movement in a given direction for a specific grid and position.
    // Returns true if any tile moved or merged.
    bool performProcessMovement(Position& pos, vector<vector<int>>& grid, char dir);


    // --- Game State and Information ---

    // Resets the game to a new configuration (start number and grid size).
    // Useful if reusing a GridGame object for multiple games (though main creates new ones).
    // Note: This function was not in the user's provided base code, but is useful for multiple games.
    // Keeping it commented out as per the instruction to stick to the provided code's functionality.
    // void resetGame(int startNumber, int gridSize);


    // Displays the current game state (both grids, move counts, status) to the console.
    void displayGameState(); // Removed const here

    // Displays a specific historical game state to the console.
    // Note: This function was not in the user's provided base code, but is useful for debugging.
    // Keeping it commented out as per the instruction to stick to the provided code's functionality.
    // void displaySpecificState(const vector<vector<int>>& g1, const vector<vector<int>>& g2) const;


    // Returns a string representation of a GameOverReason.
    string getReasonString(GameOverReason reason, int moveCount) const;


    // Checks if a grid has reached a game over state (win or stalemate).
    bool checkGameOver(const vector<vector<int>>& grid) const;

    // Checks if a grid contains the winning value (2).
    bool hasWon(const vector<vector<int>>& grid) const;

    // Checks if a grid has reached a game over state and returns the specific reason.
    GameOverReason checkGameOverReason(const vector<vector<int>>& grid) const;

    bool processMovement(Position& pos, vector<vector<int>>& grid, char dir);


    // --- Getters for Game State ---
    // Provide read-only access to important game state variables.

    const vector<vector<int>>& getGrid1() const { return grid1; } // Get AI1's grid
    const vector<vector<int>>& getGrid2() const { return grid2; } // Get AI2's grid
    int getGridSize() const { return gridSize; }                 // Get the size of the grid
    const Position& getPos1() const {
        return pos1;
        }             // Get AI1's position (if relevant)
    const Position& getPos2() const {
        return pos2;
         }             // Get AI2's position (if relevant)

    int getEmptyValue() const {
        return EMPTY;
         } // Getter for the EMPTY constant


    // --- Constants (Publicly Accessible) ---
    // Define game constants that might be needed by AI implementations.



private:

    int currentNumber;
    int gridSize;
    vector<int> possibleSpawnValues;

    // Track move counts first (to match constructor initialization)
    int Ai1Count;
    int Ai2Count;

    // Game state flags
    bool grid1GameOver;
    bool grid2GameOver;
    char lastMoveGrid1;
    char lastMoveGrid2;
    bool isFirstDisplay;
    GameOverReason grid1Reason;
    GameOverReason grid2Reason;

    // Random number generator
    mt19937 rng;

    // Grids and positions
    vector<vector<int>> grid1;
    vector<vector<int>> grid2;
    Position pos1;
    Position pos2;

    // AI interfaces
    SmartMergeMax* ai1;
    ExpectimaxAI* ai2;

    // Game history
    vector<GameStep> gameHistory;
    // --- Private Helper Methods ---

    // Initializes the possible values that can be spawned on the grid
    // based on the game's starting number.
    void initPossibleSpawnValues();

    // Validates the initial game configuration (e.g., grid size, start number).
    void validateConfiguration() const;

    // Initializes the grids with the starting tiles as per project rules.
    void initializeGrids();

    // Calculates a new position based on a starting position and a direction.
    Position calculateNewPosition(Position pos, char dir) const;

    // Checks if a given position is within the grid boundaries.
    bool isValidPosition(Position pos) const;

    // Spawns a random number from possibleSpawnValues into a random empty cell
    // on the given grid.
    void spawnRandomNumber(vector<vector<int>>& grid);

    // Converts a direction character ('w', 'a', 's', 'd', etc.) to a short string ('U', 'D', 'L', 'R').
    string directionToString(char dir) const;




    // --- Private Human Input Handling ---
    // Handles user input for moves (used in the run() loop).
    void handleInput(char input);
    // Displays controls to the user (used in the run() loop).
    void showControls() const;
};

#endif // GRIDGAME_H
