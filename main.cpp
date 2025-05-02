#include "GridGame.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

// Helper function to print a single grid to an output stream
void printGrid( ostream& os, const  vector< vector<int>>& grid, int gridSize);
// Function to write the game result to the output file
void writeGameResultToFile( ofstream& outputFile, const GameResult& result, int gameNumber) ;



int main() {

     ifstream inputFile("reverse2048.txt");

    // Open the output file in truncate mode to clear previous results
    ofstream outputFile("output.txt", ios::trunc);


    string line;
    int gameNumber = 0;


    while (getline(inputFile, line)) {
        stringstream ss(line);
        int startNumber, gridSize;

        if (ss >> startNumber >> gridSize) {
            gameNumber++;
           cout<<"game of reverse"<<startNumber<<" on a ("<<gridSize<<"x"<<gridSize<<") grid"<< endl;


                // Create a new game instance for each configuration
                GridGame game(startNumber, gridSize);

                // Run the AI game until it's over and get the result
                GameResult result = game.aiPlayUntilGameOver();

                // Write the result to the output file
                writeGameResultToFile(outputFile, result, gameNumber);

                cout << "Game " << gameNumber << " finished. Result written to output.txt" << endl;
            }
    }

    inputFile.close();
    outputFile.close();

    cout << "\nAll games processed. Results are in output.txt" <<  endl;

    return 0;
}


void printGrid( ostream& os, const  vector< vector<int>>& grid, int gridSize) {
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
};
void writeGameResultToFile( ofstream& outputFile, const GameResult& result, int gameNumber) {


    outputFile<<"========================================================================="<<endl;
    outputFile << "Game " << gameNumber << ": reverse " << result.startNumber << " (" << result.gridSize << "x" << result.gridSize << ")" ;

    // Determine and write the winner or draw outcome
    bool ai1Won = (result.grid1Reason == GameOverReason::WIN);
    bool ai2Won = (result.grid2Reason == GameOverReason::WIN);

  if(!ai1Won&&!ai2Won) {
        // Draw scenario
        outputFile << "Outcome: Draw, ";
        // Need a GridGame instance to use getReasonString, or replicate the logic here
        // Replicating the logic for simplicity in main
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

    // Print history only for winning games as per previous logic/brief example
    if (ai1Won || ai2Won) {
        for (const auto& step : result.history) {
            // Write step header (Initial Board or Move #)
            if (step.stepNumber == 0) {
                outputFile << "  Initial Board" <<  endl;
                outputFile<<"------------------------------------------------------------------------------------"<<endl;
            } else {

                // Replicating directionToString logic here for move characters
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
                    case 'w': case 'i': move2Str = "U"; break;
                    case 's': case 'k': move2Str = "D"; break;
                    case 'a': case 'j': move2Str = "L"; break;
                    case 'd': case 'l': move2Str = "R"; break;
                    default: move2Str = ""; break;
                }
outputFile << "Move " << step.stepNumber << ": Alg_1:  "<< move1Str << setw(result.gridSize)<<"     "<<"             Alg_2: " << move2Str <<  endl;
            }

            // Print grids side-by-side with displayGameState formatting
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

                outputFile << "\n"; // Newline after each row
            }
            // Print separator line after the grid
            outputFile << "-----------------------------------------------------------------------\n";
        }
    }
       if (ai1Won && !ai2Won) {
        outputFile << "Winner: Alg 1, total moves " << result.ai1MoveCount <<  endl;
    } else if (!ai1Won && ai2Won) {
        outputFile << "Winner: Alg 2, total moves " << result.ai2MoveCount <<  endl;
    } else if (ai1Won && ai2Won) {
        // Both won - determine winner by move count
        if (result.ai1MoveCount <= result.ai2MoveCount) {
            outputFile << "Winner: Alg 1 (reached win in " << result.ai1MoveCount << " moves)" <<  endl;
        } else {
            outputFile << "Winner: Alg 2 (reached win in " << result.ai2MoveCount << " moves)" <<  endl;
        }
    }
    // Add a blank line after each game's output
    outputFile <<  endl;
}
