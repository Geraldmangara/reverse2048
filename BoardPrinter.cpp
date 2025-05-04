#include "BoardPrinter.h"
#include <sstream>
void BoardPrinter::printGrid( ostream& os, const  vector< vector<int>>& grid, int gridSize)
{
    for (int i = 0; i < gridSize; ++i) {
        os << "|";
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j] == GridGame::EMPTY)
                os <<  setw(5) << " - ";
            else
                os <<  setw(5) << grid[i][j];
        }
        os << " |";
    }
}
void BoardPrinter::writeGameResultToFile( ofstream& outputFile, const GameResult& result, int gameNumber)
{
    outputFile << "=========================================================================" <<  endl;
    outputFile << "Game " << gameNumber << ": reverse " << result.startNumber
               << " (" << result.gridSize << "x" << result.gridSize << ")";

    // Determine and write the winner or draw outcome
    bool ai1Won = (result.grid1Reason == GameOverReason::WIN);
    bool ai2Won = (result.grid2Reason == GameOverReason::WIN);

    if (!ai1Won && !ai2Won) {
        // Draw scenario
        outputFile << "Outcome: Draw, ";
        // Replicating the logic for simplicity
         string reason1Str, reason2Str;

        switch (result.grid1Reason) {
            case GameOverReason::STALEMATE_BOARD_FULL: reason1Str = "Board Full"; break;
            case GameOverReason::STALEMATE_NO_MOVES: reason1Str = "No Valid Moves"; break;
            case GameOverReason::MOVE_LIMIT_REACHED: reason1Str = "Move Limit Reached (" +  to_string(result.ai1MoveCount) + " moves)"; break;
            case GameOverReason::OPPONENT_WON: reason1Str = "Opponent Won"; break;
            default: reason1Str = "Unknown"; break;
        }
       switch (result.grid2Reason) {
            case GameOverReason::STALEMATE_BOARD_FULL: reason2Str = "Board Full"; break;
            case GameOverReason::STALEMATE_NO_MOVES: reason2Str = "No Valid Moves"; break;
            case GameOverReason::MOVE_LIMIT_REACHED: reason2Str = "Move Limit Reached (" +  to_string(result.ai2MoveCount) + " moves)"; break;
            case GameOverReason::OPPONENT_WON: reason2Str = "Opponent Won"; break;
            default: reason2Str = "Unknown"; break;
        }

        outputFile << "Alg 1: " << reason1Str;
        outputFile << " | Alg 2: " << reason2Str <<  endl;
    }

    // Print history only for winning games 
if (ai1Won || ai2Won) {
        for (const auto& step : result.history) {
            // Write step header (Initial Board or Move #)
            if (step.stepNumber == 0) {
                outputFile << "  Initial Board" <<  endl;
                outputFile << "------------------------------------------------------------------------------------" <<  endl;
            } 
else {
                // directionToString logic here for move characters
                char move1 = tolower(step.move1);
                char move2 = tolower(step.move2);
                 string move1Str, move2Str;

                switch(move1) {
                    case 'w': case 'i': move1Str = "U"; break;
                    case 's': case 'k': move1Str = "D"; break;
                    case 'a': case 'j': move1Str = "L"; break;
                    case 'd': case 'l': move1Str = "R"; break;
                    default: move1Str = ""; break;
                }

                switch(move2) {
                    case 'w': case 'i': move2Str = "U";break;
                    case 's': case 'k': move2Str = "D"; break;
                    case 'a': case 'j': move2Str = "L"; break;
                    case 'd': case 'l': move2Str = "R"; break;
                    default: move2Str = ""; break;
                }

                outputFile << "Move " << step.stepNumber << ": Alg_1:  " << move1Str
                           <<  setw(result.gridSize) << "     " << "             Alg_2: " << move2Str <<  endl;
            }
            for (int i = 0; i < result.gridSize; ++i) {
                // Grid 1
                outputFile << "|";
                for (int j = 0; j < result.gridSize; ++j) {
                    if (step.grid1State[i][j] == GridGame::EMPTY)
                        outputFile <<  setw(5) << " - ";
                    else
                        outputFile <<  setw(5) << step.grid1State[i][j];
                }
                outputFile << " |"; // Close Grid 1

                outputFile << "     "; // Space between the two grids

                // Grid 2
                outputFile << "|";
                for (int j = 0; j < result.gridSize; ++j) {
                    if (step.grid2State[i][j] == GridGame::EMPTY)
                        outputFile <<  setw(5) << " - ";
                    else
                        outputFile <<  setw(5) << step.grid2State[i][j];
                }
                outputFile << " |"; // Close Grid 2

                outputFile << "\n"; 
            }
        
            outputFile << "-----------------------------------------------------------------------\n";
        }
    }
if (ai1Won && !ai2Won) {
        outputFile << "Winner: Alg 1, total moves " << result.ai1MoveCount <<  endl;
    } 
else if (!ai1Won && ai2Won) {
        outputFile << "Winner: Alg 2, total moves " << result.ai2MoveCount <<  endl;
    }
else if (ai1Won && ai2Won) {
        // Because the game will run both Ais until completion, and for testing conditions, print out a winner by the move number
 if (result.ai1MoveCount <= result.ai2MoveCount) {
            outputFile << "Winner: Alg 1 (reached win in " << result.ai1MoveCount << " moves)" <<  endl;
        }
 else {
    outputFile << "Winner: Alg 2 (reached win in " << result.ai2MoveCount << " moves)" <<  endl;
        }
    }
    
    outputFile <<  endl;
}
