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


using namespace std;


class SmartMergeMax;
class ExpectimaxAI;

struct Position {
    int row; 
    int col; 
  Position() : row(0), col(0) {}
  Position(int r, int c) : row(r), col(c) {}
   bool operator==(const Position& other) const {
  return row == other.row && col == other.col;
    }

    // An operator for inequality, thought it was useful 
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

// Enum to specify the reason for a game over state. Used throughout game logic. It is the rule enforcer
enum GameOverReason {
    NOT_OVER,            
    WIN,                 
    STALEMATE_BOARD_FULL, 
    STALEMATE_NO_MOVES,  
    MOVE_LIMIT_REACHED,  
    OPPONENT_WON         
};

/* Pretty powerful struct, holds game history. As outputting to an output file requires saved data, this struct and the objects of GameResult
     are essential to the codebase */
struct GameStep {
    vector<vector<int>> grid1State; 
    vector<vector<int>> grid2State; 
    char move1;                    
    char move2;                   
    int stepNumber;             

    // Default constructor, just contains moves and step number
    GameStep() : move1(' '), move2(' '), stepNumber(0) {}
    // Parameterised constructor, contains everything needed for game history. A decent use of Object Oriented Programming
    GameStep(const vector<vector<int>>& g1, const vector<vector<int>>& g2, char m1, char m2, int step)
        : grid1State(g1), grid2State(g2), move1(m1), move2(m2), stepNumber(step) {}
};

// Structure to hold the result of a game
struct GameResult {
    int startNumber;        
    int gridSize;
    GameOverReason grid1Reason;
    GameOverReason grid2Reason; 
    int ai1MoveCount;       
    int ai2MoveCount;       
    vector<GameStep> history; 

    // Default constructor
    GameResult() : startNumber(0), gridSize(0), grid1Reason(GameOverReason::NOT_OVER),
                   grid2Reason(GameOverReason::NOT_OVER), ai1MoveCount(0), ai2MoveCount(0) {}

    //Parameterised constructor
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
    // Constructor
    GridGame(int startNumber, int gridSize);
   ~GridGame();
   GameResult aiPlayUntilGameOver(); 
    bool ai1PlayOneStep();
    bool ai2PlayOneStep();
    bool aiPlayOneStep();

    // Public wrapper for RrocessMovement, just for the AIs
    bool performProcessMovement(Position& pos, vector<vector<int>>& grid, char dir);
    string getReasonString(GameOverReason reason, int moveCount) const;
    bool checkGameOver(const vector<vector<int>>& grid) const;
    bool hasWon(const vector<vector<int>>& grid) const;
    GameOverReason checkGameOverReason(const vector<vector<int>>& grid) const;

    bool processMovement(Position& pos, vector<vector<int>>& grid, char dir);

//  Some getters
    const vector<vector<int>>& getGrid1() const { return grid1; } 
    const vector<vector<int>>& getGrid2() const { return grid2; } 
    int getGridSize() const { return gridSize; }                
    const Position& getPos1() const {
        return pos1;
        }      
    const Position& getPos2() const {
        return pos2;
         }       
    int getEmptyValue() const {
        return EMPTY;
         } 

private:
    int currentNumber;
    int gridSize;
    vector<int> possibleSpawnValues;
    // move tracker
    int Ai1Count;
    int Ai2Count;

    // Game state flags->with enums become a powerful combination
    bool grid1GameOver;
    bool grid2GameOver;
    char lastMoveGrid1;
    char lastMoveGrid2;
    bool isFirstDisplay;
    GameOverReason grid1Reason;
    GameOverReason grid2Reason;

    // Random number generator
    mt19937 rng;
    vector<vector<int>> grid1;
    vector<vector<int>> grid2;
    Position pos1;
    Position pos2;

    // AI interfaces
    SmartMergeMax* ai1;
    ExpectimaxAI* ai2;
    vector<GameStep> gameHistory;
    // Helper stuff
    void initPossibleSpawnValues();
    void validateConfiguration() const;
   void initialiseGrids();
    Position calculateNewPosition(Position pos, char dir) const;
    bool isValidPosition(Position pos) const;
    void spawnRandomNumber(vector<vector<int>>& grid);
    string directionToString(char dir) const;

};

#endif 
