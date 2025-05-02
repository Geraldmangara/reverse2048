#include "AlgoTester.h"
#include "GridGame.h"
#include "SmartMergeMax.h"
#include "ExpectimaxAI.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <numeric>
#include <cmath>

using namespace std;

// Constructor
AlgoTester::AlgoTester(const string& algo1Name, const string& algo2Name)
    : algo1Name(algo1Name), algo2Name(algo2Name) {
}

// Destructor
AlgoTester::~AlgoTester() {
}

// Run a batch of tests with specified parameters
vector<GameTestResult> AlgoTester::runTests(
    int numGames,
    int startNumber,
    int gridSize,
    TestType testType) {

    vector<GameTestResult> results;

    cout << "Running " << numGames << " games with start number " << startNumber
         << " and grid size " << gridSize << " (" << testTypeToString(testType) << ")" << endl;

    for (int i = 0; i < numGames; ++i) {
        cout << "Running game " << (i + 1) << " of " << numGames << "... ";
        GameTestResult result = runSingleTest(i + 1, startNumber, gridSize, testType);
        results.push_back(result);
        cout << "done! Winner: " << result.winnerName << endl;
    }

    return results;
}

// Run tests with multiple configurations
map<tuple<int, int>, vector<GameTestResult>> AlgoTester::runMultipleTests(
    int numGames,
    const vector<int>& startNumbers,
    const vector<int>& gridSizes,
    TestType testType) {

    map<tuple<int, int>, vector<GameTestResult>> allResults;

    for (int startNumber : startNumbers) {
        for (int gridSize : gridSizes) {
            tuple<int, int> config = make_tuple(startNumber, gridSize);
            cout << "\n==========================================\n";
            cout << "Testing configuration: start=" << startNumber << ", grid=" << gridSize << endl;
            cout << "==========================================\n";

            vector<GameTestResult> results = runTests(numGames, startNumber, gridSize, testType);
            allResults[config] = results;
        }
    }

    return allResults;
}

// Run a single test game and return detailed results
GameTestResult AlgoTester::runSingleTest(int gameNumber, int startNumber, int gridSize, TestType testType) {
    // Create a new game instance
    GridGame game(startNumber, gridSize);

    // Variables to track time
    chrono::high_resolution_clock::time_point startTime1, endTime1, startTime2, endTime2;
    double totalTime1 = 0.0, totalTime2 = 0.0;
    int moveCount1 = 0, moveCount2 = 0;
    bool gameOver = false;

    // If we're testing time per move, we need to manually control the game flow
    if (testType == TestType::TIME_PER_MOVE) {
        // Store the initial state for history
        vector<GameStep> gameHistory;
        gameHistory.push_back({game.getGrid1(), game.getGrid2(), ' ', ' ', 0});

        // Play the game step by step, tracking time
        while (!gameOver) {
            // AI1's turn
            if (!gameOver) {
                startTime1 = chrono::high_resolution_clock::now();
                bool moved1 = game.ai1PlayOneStep();
                endTime1 = chrono::high_resolution_clock::now();

                if (moved1) {
                    moveCount1++;
                    double moveTime = chrono::duration_cast<chrono::microseconds>(endTime1 - startTime1).count() / 1000.0;
                    totalTime1 += moveTime;

                    // Add step to history
                    gameHistory.push_back({game.getGrid1(), game.getGrid2(), 'X', ' ', (int)gameHistory.size()});
                }
            }

            // AI2's turn
            if (!gameOver) {
                startTime2 = chrono::high_resolution_clock::now();
                bool moved2 = game.ai2PlayOneStep();
                endTime2 = chrono::high_resolution_clock::now();

                if (moved2) {
                    moveCount2++;
                    double moveTime = chrono::duration_cast<chrono::microseconds>(endTime2 - startTime2).count() / 1000.0;
                    totalTime2 += moveTime;

                    // Add step to history
                    gameHistory.push_back({game.getGrid1(), game.getGrid2(), ' ', 'X', (int)gameHistory.size()});
                }
            }

            // Check if game is over
            gameOver = game.checkGameOver(game.getGrid1()) && game.checkGameOver(game.getGrid2());
        }

        // Get final game state
        GameOverReason reason1 = game.checkGameOverReason(game.getGrid1());
        GameOverReason reason2 = game.checkGameOverReason(game.getGrid2());

        // Determine winner
        bool algo1Win = (reason1 == GameOverReason::WIN);
        bool algo2Win = (reason2 == GameOverReason::WIN);
        string winnerName = "Draw";

        if (algo1Win && !algo2Win) {
            winnerName = algo1Name;
        } else if (!algo1Win && algo2Win) {
            winnerName = algo2Name;
        } else if (algo1Win && algo2Win) {
            // Both won, determine by move count
            if (moveCount1 <= moveCount2) {
                winnerName = algo1Name + " (by moves)";
            } else {
                winnerName = algo2Name + " (by moves)";
            }
        }

        // Create and return result
        GameTestResult result;
        result.gameNumber = gameNumber;
        result.startNumber = startNumber;
        result.gridSize = gridSize;
        result.algo1Reason = reason1;
        result.algo2Reason = reason2;
        result.algo1MoveCount = moveCount1;
        result.algo2MoveCount = moveCount2;
        result.algo1TotalTime = totalTime1;
        result.algo2TotalTime = totalTime2;
        result.algo1Win = algo1Win;
        result.algo2Win = algo2Win;
        result.winnerName = winnerName;

        return result;
    } else {
        // For other test types, we can use the existing aiPlayUntilGameOver method
        GameResult gameResult = game.aiPlayUntilGameOver();

        // Extract data from the game result
        bool algo1Win = (gameResult.grid1Reason == GameOverReason::WIN);
        bool algo2Win = (gameResult.grid2Reason == GameOverReason::WIN);
        string winnerName = "Draw";

        if (algo1Win && !algo2Win) {
            winnerName = algo1Name;
        } else if (!algo1Win && algo2Win) {
            winnerName = algo2Name;
        } else if (algo1Win && algo2Win) {
            // Both won, determine by move count
            if (gameResult.ai1MoveCount <= gameResult.ai2MoveCount) {
                winnerName = algo1Name + " (by moves)";
            } else {
                winnerName = algo2Name + " (by moves)";
            }
        }

        // Create and return result
        GameTestResult result;
        result.gameNumber = gameNumber;
        result.startNumber = gameResult.startNumber;
        result.gridSize = gameResult.gridSize;
        result.algo1Reason = gameResult.grid1Reason;
        result.algo2Reason = gameResult.grid2Reason;
        result.algo1MoveCount = gameResult.ai1MoveCount;
        result.algo2MoveCount = gameResult.ai2MoveCount;
        result.algo1TotalTime = 0.0; // Not measured
        result.algo2TotalTime = 0.0; // Not measured
        result.algo1Win = algo1Win;
        result.algo2Win = algo2Win;
        result.winnerName = winnerName;

        return result;
    }
}

// Generate summary statistics from results
TestSummary AlgoTester::generateSummary(const vector<GameTestResult>& results) {
    TestSummary summary;

    // Initialize counters
    summary.gamesPlayed = results.size();
    summary.algo1Wins = 0;
    summary.algo2Wins = 0;
    summary.draws = 0;

    // Sum up values
    int totalAlgo1Moves = 0;
    int totalAlgo2Moves = 0;
    double totalAlgo1Time = 0.0;
    double totalAlgo2Time = 0.0;

    for (const auto& result : results) {
        if (result.algo1Win && !result.algo2Win) {
            summary.algo1Wins++;
        } else if (!result.algo1Win && result.algo2Win) {
            summary.algo2Wins++;
        } else if (result.algo1Win && result.algo2Win) {
            // Both won, determine by move count
            if (result.algo1MoveCount <= result.algo2MoveCount) {
                summary.algo1Wins++;
            } else {
                summary.algo2Wins++;
            }
        } else {
            summary.draws++;
        }

        totalAlgo1Moves += result.algo1MoveCount;
        totalAlgo2Moves += result.algo2MoveCount;
        totalAlgo1Time += result.algo1TotalTime;
        totalAlgo2Time += result.algo2TotalTime;
    }

    // Calculate averages
    summary.algo1WinRate = static_cast<double>(summary.algo1Wins) / summary.gamesPlayed;
    summary.algo2WinRate = static_cast<double>(summary.algo2Wins) / summary.gamesPlayed;
    summary.avgAlgo1Moves = static_cast<double>(totalAlgo1Moves) / summary.gamesPlayed;
    summary.avgAlgo2Moves = static_cast<double>(totalAlgo2Moves) / summary.gamesPlayed;

    // Calculate average time per move
    if (totalAlgo1Moves > 0) {
        summary.avgAlgo1TimePerMove = totalAlgo1Time / totalAlgo1Moves;
    } else {
        summary.avgAlgo1TimePerMove = 0.0;
    }

    if (totalAlgo2Moves > 0) {
        summary.avgAlgo2TimePerMove = totalAlgo2Time / totalAlgo2Moves;
    } else {
        summary.avgAlgo2TimePerMove = 0.0;
    }

    return summary;
}

// Generate CSV file with detailed results
void AlgoTester::exportResultsToCSV(const vector<GameTestResult>& results, const string& filename) {
    ofstream csvFile(filename);

    if (!csvFile.is_open()) {
        cerr << "Error: Could not open file " << filename << " for writing." << endl;
        return;
    }

    // Write CSV header
    csvFile << "Game,StartNumber,GridSize,"
            << algo1Name << "_Reason," << algo2Name << "_Reason,"
            << algo1Name << "_MoveCount," << algo2Name << "_MoveCount,"
            << algo1Name << "_TotalTime(ms)," << algo2Name << "_TotalTime(ms),"
            << algo1Name << "_Win," << algo2Name << "_Win,Winner" << endl;

    // Write data rows
    for (const auto& result : results) {
        csvFile << result.gameNumber << ","
                << result.startNumber << ","
                << result.gridSize << ","
                << formatReasonString(result.algo1Reason, result.algo1MoveCount) << ","
                << formatReasonString(result.algo2Reason, result.algo2MoveCount) << ","
                << result.algo1MoveCount << ","
                << result.algo2MoveCount << ","
                << fixed << setprecision(3) << result.algo1TotalTime << ","
                << fixed << setprecision(3) << result.algo2TotalTime << ","
                << (result.algo1Win ? "1" : "0") << ","
                << (result.algo2Win ? "1" : "0") << ","
                << result.winnerName << endl;
    }

    csvFile.close();
    cout << "Results exported to " << filename << endl;
}

// Generate CSV file with summary statistics
void AlgoTester::exportSummaryToCSV(
    const map<tuple<int, int>, TestSummary>& summaries,
    const string& filename) {

    ofstream csvFile(filename);

    if (!csvFile.is_open()) {
        cerr << "Error: Could not open file " << filename << " for writing." << endl;
        return;
    }

    // Write CSV header
    csvFile << "StartNumber,GridSize,GamesPlayed,"
            << algo1Name << "_Wins," << algo2Name << "_Wins,Draws,"
            << algo1Name << "_WinRate," << algo2Name << "_WinRate,"
            << algo1Name << "_AvgMoves," << algo2Name << "_AvgMoves,"
            << algo1Name << "_AvgTimePerMove(ms)," << algo2Name << "_AvgTimePerMove(ms)" << endl;

    // Write data rows
    for (const auto& pair : summaries) {
        int startNumber = get<0>(pair.first);
        int gridSize = get<1>(pair.first);
        const TestSummary& summary = pair.second;

        csvFile << startNumber << ","
                << gridSize << ","
                << summary.gamesPlayed << ","
                << summary.algo1Wins << ","
                << summary.algo2Wins << ","
                << summary.draws << ","
                << fixed << setprecision(3) << summary.algo1WinRate << ","
                << fixed << setprecision(3) << summary.algo2WinRate << ","
                << fixed << setprecision(2) << summary.avgAlgo1Moves << ","
                << fixed << setprecision(2) << summary.avgAlgo2Moves << ","
                << fixed << setprecision(3) << summary.avgAlgo1TimePerMove << ","
                << fixed << setprecision(3) << summary.avgAlgo2TimePerMove << endl;
    }

    csvFile.close();
    cout << "Summary statistics exported to " << filename << endl;
}

// Print results to console
void AlgoTester::printResults(const vector<GameTestResult>& results) {
    cout << "\n========== Detailed Results ==========\n";
    cout << left << setw(5) << "Game"
         << setw(7) << "Start"
         << setw(5) << "Grid"
         << setw(15) << algo1Name
         << setw(15) << algo2Name
         << setw(7) << "Moves1"
         << setw(7) << "Moves2"
         << setw(10) << "Time1(ms)"
         << setw(10) << "Time2(ms)"
         << "Winner" << endl;

    cout << string(90, '-') << endl;

    for (const auto& result : results) {
        cout << left << setw(5) << result.gameNumber
             << setw(7) << result.startNumber
             << setw(5) << result.gridSize
             << setw(15) << formatReasonString(result.algo1Reason, result.algo1MoveCount)
             << setw(15) << formatReasonString(result.algo2Reason, result.algo2MoveCount)
             << setw(7) << result.algo1MoveCount
             << setw(7) << result.algo2MoveCount
             << setw(10) << fixed << setprecision(2) << result.algo1TotalTime
             << setw(10) << fixed << setprecision(2) << result.algo2TotalTime
             << result.winnerName << endl;
    }

    cout << string(90, '-') << endl;
}

// Print summary to console
void AlgoTester::printSummary(const TestSummary& summary) {
    cout << "\n========== Summary Statistics ==========\n";
    cout << "Games Played:              " << summary.gamesPlayed << endl;
    cout << algo1Name << " Wins:               " << summary.algo1Wins
         << " (" << fixed << setprecision(1) << (summary.algo1WinRate * 100) << "%)" << endl;
    cout << algo2Name << " Wins:               " << summary.algo2Wins
         << " (" << fixed << setprecision(1) << (summary.algo2WinRate * 100) << "%)" << endl;
    cout << "Draws:                     " << summary.draws << endl;
    cout << algo1Name << " Average Moves:      " << fixed << setprecision(2) << summary.avgAlgo1Moves << endl;
    cout << algo2Name << " Average Moves:      " << fixed << setprecision(2) << summary.avgAlgo2Moves << endl;

    if (summary.avgAlgo1TimePerMove > 0.0) {
        cout << algo1Name << " Avg Time/Move (ms): " << fixed << setprecision(3) << summary.avgAlgo1TimePerMove << endl;
    }

    if (summary.avgAlgo2TimePerMove > 0.0) {
        cout << algo2Name << " Avg Time/Move (ms): " << fixed << setprecision(3) << summary.avgAlgo2TimePerMove << endl;
    }

    cout << "======================================\n";
}

// Helper method to get a string representation of TestType
string AlgoTester::testTypeToString(TestType testType) {
    switch (testType) {
        case TestType::COMPETITIVE:
            return "Competitive";
        case TestType::MOVE_COUNT:
            return "Move Count";
        case TestType::TIME_PER_MOVE:
            return "Time Per Move";
        default:
            return "Unknown";
    }
}

// Helper method to format a reason string
string AlgoTester::formatReasonString(GameOverReason reason, int moveCount) {
    switch (reason) {
        case GameOverReason::NOT_OVER:
            return "Not Over";
        case GameOverReason::WIN:
            return "Win";
        case GameOverReason::STALEMATE_BOARD_FULL:
            return "Board Full";
        case GameOverReason::STALEMATE_NO_MOVES:
            return "No Moves";
        case GameOverReason::MOVE_LIMIT_REACHED:
            return "Move Limit";
        case GameOverReason::OPPONENT_WON:
            return "Opponent Won";
        default:
            return "Unknown";
    }
}
