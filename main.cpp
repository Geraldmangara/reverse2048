#include "GridGame.h"
#include "BoardPrinter.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;
int main() {
     ifstream inputFile("reverse2048.txt");

    // Open the output file in truncate mode to clear previous results
     ofstream outputFile("output.txt",  ios::trunc);

     string line;
    int gameNumber = 0;

    while (getline(inputFile, line)) {
         stringstream ss(line);
        int startNumber, gridSize;

        if (ss >> startNumber >> gridSize) {
            gameNumber++;
             cout << "game of reverse" << startNumber << " on a (" << gridSize << "x" << gridSize << ") grid" <<  endl;

            // Create a new game instance for each configuration
            GridGame game(startNumber, gridSize);

            // Run the AI game until it's over and get the result
            GameResult result = game.aiPlayUntilGameOver();

            // Write the result to the output file using the BoardPrinter class
            BoardPrinter::writeGameResultToFile(outputFile, result, gameNumber);

             cout << "Game " << gameNumber << " finished. Result written to output.txt" <<  endl;
        }
    }

    inputFile.close();
    outputFile.close();

     cout << "\nAll games processed. Results are in output.txt" <<  endl;

    return 0;
}
