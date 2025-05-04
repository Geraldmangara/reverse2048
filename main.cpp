/* 2804713
   2844414
   A simple solution to reverse2048.
*/


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
     ifstream inputFile("input.txt");
     ofstream outputFile("output.txt");

     string line;
    int gameNumber = 0;

    while (getline(inputFile, line)) {
         stringstream ss(line);
        int startNumber, gridSize;
        char s;
        if (ss >> startNumber>>s >> gridSize) {
            gameNumber++;
            GridGame game(startNumber, gridSize);
            GameResult result = game.aiPlayUntilGameOver();
            BoardPrinter::writeGameResultToFile(outputFile, result, gameNumber);
        }
    }
    inputFile.close();
    outputFile.close();
    return 0;
}

// thank you for reviewing the code.Hope it met all your expectations
