#include "GridGame.h"
#include "ExpectimaxAI.h"

// Initialize possible spawn values based on the starting number
void GridGame::initPossibleSpawnValues() {
    possibleSpawnValues.clear();
    if (currentNumber == 512) {
        possibleSpawnValues = {256, 128, 64};
    } else if (currentNumber == 256) {
        possibleSpawnValues = {128, 64, 32};
    } else if (currentNumber == 128) {
        possibleSpawnValues = {64, 32, 16};
    } else {
        // Default case - just use the currentNumber
        possibleSpawnValues = {currentNumber};
    }
}

// Checks if config file values are acceptable
void GridGame::validateConfiguration() const {
    if (gridSize < MIN_GRID_SIZE || gridSize > MAX_GRID_SIZE) {
        throw invalid_argument("Grid size must be between 3 and 5");
    }
    if (find(VALID_NUMBERS.begin(), VALID_NUMBERS.end(), currentNumber) == VALID_NUMBERS.end()) {
        throw invalid_argument("Number must be 128, 256, or 512");
    }
}

// Prepares both grids and adds two numbers each
void GridGame::initializeGrids() {
    grid1 = vector<vector<int>>(gridSize, vector<int>(gridSize, EMPTY));
    grid2 = vector<vector<int>>(gridSize, vector<int>(gridSize, EMPTY));

    // Initialize possible spawn values
    initPossibleSpawnValues();

    // Random positions for the first two numbers on Grid 1
    uniform_int_distribution<int> dist(0, gridSize - 1);
    int row1a, col1a, row1b, col1b;


    do {
        row1a = dist(rng);
        col1a = dist(rng);
        row1b = dist(rng);
        col1b = dist(rng);
    } while (row1a == row1b && col1a == col1b); // Ensure they’re not the same cell

    grid1[row1a][col1a] = currentNumber;
    grid1[row1b][col1b] = currentNumber;
    grid2[row1a][col1a] = currentNumber;
    grid2[row1b][col1b] = currentNumber;



}

// Figures out where the cursor would move given a direction
Position GridGame::calculateNewPosition(Position pos, char dir) const {
    dir = tolower(dir);
    Position newPos = pos;
    switch (dir) {
    case 'w':
    case 'i':
        newPos.row--;
        break;
    case 's':
    case 'k':
        newPos.row++;
        break;
    case 'a':
    case 'j':
        newPos.col--;
        break;
    case 'd':
    case 'l':
        newPos.col++;
        break;
    }
    return newPos;
}

// Checks if a given grid position is within bounds
bool GridGame::isValidPosition(Position pos) const {
    return pos.row >= 0 && pos.row < gridSize &&
           pos.col >= 0 && pos.col < gridSize;
}

// Adds a new tile with a random value from possibleSpawnValues in an empty spot
void GridGame::spawnRandomNumber(vector<vector<int>>& grid) {
    vector<Position> emptyCells;
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (grid[i][j] == EMPTY) {
                emptyCells.push_back({i, j});
            }
        }
    }

    if (!emptyCells.empty()) {
        uniform_int_distribution<int> cellDist(0, emptyCells.size() - 1);
        Position spawnPos = emptyCells[cellDist(rng)];

        // Choose a random value from possibleSpawnValues
        uniform_int_distribution<int> valueDist(0, possibleSpawnValues.size() - 1);
        int valueIndex = valueDist(rng);

        grid[spawnPos.row][spawnPos.col] = possibleSpawnValues[valueIndex];
    }
}

// Moves tiles in a given direction and handles merging
bool GridGame::processMovement(Position& pos, vector<vector<int>>& grid, char dir) {
    bool gridChanged = false;
    dir = tolower(dir);

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
                    if (grid[newRow][newCol] == grid[row][col]) {
                        grid[newRow][newCol] /= 2;
                        grid[row][col] = EMPTY;
                    } else {
                        grid[newRow][newCol] = grid[row][col];
                        grid[row][col] = EMPTY;
                    }
                    gridChanged = true;
                }
            }
        }
    }

    if (gridChanged) {
        Position newPos = calculateNewPosition(pos, dir);
        if (isValidPosition(newPos)) {
            pos = newPos;
        }
        spawnRandomNumber(grid);
        return true;
    }

    return false;
}

// Checks if the game has reached a win or stalemate for a grid
bool GridGame::checkGameOver(const vector<vector<int>>& grid) const {
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (grid[i][j] == 2) {
                return true;
            }
        }
    }

    bool gridFull = true;
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (grid[i][j] == EMPTY) {
                gridFull = false;
                break;
            }
        }
        if (!gridFull) break;
    }

    if (gridFull) {
        for (int i = 0; i < gridSize; i++) {
            for (int j = 0; j < gridSize - 1; j++) {
                if (grid[i][j] == grid[i][j+1]) {
                    return false;
                }
            }
        }

        for (int j = 0; j < gridSize; j++) {
            for (int i = 0; i < gridSize - 1; i++) {
                if (grid[i][j] == grid[i+1][j]) {
                    return false;
                }
            }
        }

        return true;
    }

    return false;
}

// Constructor that loads config and starts game
GridGame::GridGame(const string& InputFile) : rng(random_device()()) {
    ifstream file(InputFile);
    if(!file) {
        cout<<"Error: Please input the right config file. Look at the documentation and the readmefile(reverse2048)!"<<endl;
    }

    file >> currentNumber >> gridSize;
    validateConfiguration();
    initializeGrids();
    Ai2Count=0;
    pos1={0,0};
    pos2={0,0};

    // the biggest line of code. My magnum opus
    ai = new ExpectimaxAI(grid2, pos2, gridSize, currentNumber, 7, EMPTY);
}

// Destructor to clean up the AI
GridGame::~GridGame() {
    if (ai) delete ai;
}

void GridGame::handleInput(char input) {
    input = tolower(input);
    bool gridChanged = false;

    const string grid1Controls = "wasd";
    const string grid2Controls = "ijkl";

    if (input == 'q') return;

    if (grid1Controls.find(input) != string::npos) {
        gridChanged = processMovement(pos1, grid1, input);

        if (checkGameOver(grid1)) {
            displayGameState();
            cout << "\nGame Over for Grid 1! ";
            for (int i = 0; i < gridSize; i++) {
                for (int j = 0; j < gridSize; j++) {
                    if (grid1[i][j] == 2) {
                        cout << "You won by reaching 2!\n";
                        return;
                    }
                }
            }
            cout << "No more moves possible.\n";
            return;
        }
    } else if (grid2Controls.find(input) != string::npos) {
        gridChanged = processMovement(pos2, grid2, input);

        if (checkGameOver(grid2)) {
            displayGameState();
            cout << "\nGame Over for Grid 2! ";
            for (int i = 0; i < gridSize; i++) {
                for (int j = 0; j < gridSize; j++) {
                    if (grid2[i][j] == 2) {
                        cout << "AI won by reaching 2!\n";
                        return;
                    }
                }
            }
            cout << "No more moves possible for AI.\n";
            return;
        }
    }

    if (gridChanged) {
        displayGameState();
    } else if (input != 'q') {
        cout << "Invalid move!\n";
    }
}

// Prints out both grids side-by-side with current number
void GridGame::displayGameState() const {
    // Header
    cout << "---------------------------------------------------------------------\n";
    cout << "Game 1: reverse 512 (" << gridSize << " x " << gridSize << ") - Initial Board\n";
    cout << "---------------------------------------------------------------------\n";

    // Display both grids side by side
    for (int row = 0; row < gridSize; ++row) {
        // Grid 1 (Human)
        cout << "|";
        for (int col = 0; col < gridSize; ++col) {
            if (grid1[row][col] == EMPTY)
                cout << setw(5) << " - ";
            else
                cout << setw(5) << grid1[row][col];
        }
        cout << " |"; // Close Grid 1

        cout << "     "; // Space between the two grids

        // Grid 2 (AI)
        cout << "|";
        for (int col = 0; col < gridSize; ++col) {
            if (grid2[row][col] == EMPTY)
                cout << setw(5) << " - ";
            else
                cout << setw(5) << grid2[row][col];
        }
        cout << " |"; // Close Grid 2

        cout << "\n";
    }

    // Bottom border
    cout <<"---------------------------------------------------------------------\n";
}


// Shows movement keys to the player
void GridGame::showControls() const {
    cout << "\nControls:\n"
         << "WASD - Move Grid 1 (Human)\n"
         << "IJKL - Move Grid 2 (AI is controlling this grid)\n"
         << "Q - Quit\n"
         << "A - Let AI play one step\n"
         << "P - Let AI play until game over\n\n";
}

// Let the AI play one step
bool GridGame::aiPlayOneStep() {
    ai->resetCache(); // cashing system, resets after a move to get new calculations
    return ai->playOneStep(this);
}

// Let the AI play until game over
void GridGame::aiPlayUntilGameOver() {
    cout << "AI playing automatically. Press Ctrl+C to stop.\n";

    bool gameEnded = false;
    Ai2Count=0;
    while (!gameEnded) {
        ai->resetCache();
        bool validMove = ai->playOneStep(this);
        if(validMove){
            Ai2Count++;
        }

        if (!validMove) {
            cout << "AI has no valid moves left.\n";
            gameEnded = true;
        }

        if (checkGameOver(grid2)) {
            displayGameState();
            cout << "\nGame Over for Grid 2 (AI)! ";
            for (int i = 0; i < gridSize; i++) {
                for (int j = 0; j < gridSize; j++) {
                    if (grid2[i][j] == 2) {
                        cout << "I love beating idiots like you \n";
                        cout << " Expectimax Ai beat you with "<<Ai2Count<<" moves.";
                        return;
                    }
                }
            }
            cout << "No more moves possible.\n";
            return;
        }

        // delay stuff
        this_thread::sleep_for(chrono::milliseconds(300));
    }
}

// Starts the main game loop
void GridGame::run() {
    showControls();
    displayGameState();

    char input;
    do {
        cout << "> ";
        cin >> input;

        input = tolower(input);

        // input stuff
        if (input == 'a') {
            aiPlayOneStep();
        } else if (input =='p') {
            aiPlayUntilGameOver();
        } else {
            handleInput(input);
        }
    } while (tolower(input) != 'q');
}


bool GridGame::performProcessMovement(Position& pos, vector<vector<int>>& grid, char dir) {
    return processMovement(pos, grid, dir);
}
