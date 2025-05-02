#ifndef BOARDPRINTER_H
#define BOARDPRINTER_H

#include "GridGame.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;

class BoardPrinter {
public:
    // Static method to print a single grid to an output stream
    static void printGrid( ostream& os, const  vector< vector<int>>& grid, int gridSize);

    // Static method to write the game result to the output file
    static void writeGameResultToFile( ofstream& outputFile, const GameResult& result, int gameNumber);
};

#endif // BOARDPRINTER_H
