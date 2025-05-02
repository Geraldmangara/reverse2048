#ifndef ALGO_TESTER_H
#define ALGO_TESTER_H

#include "GridGame.h"
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <map>
#include <tuple>

// Forward declarations
class ExpectimaxAI;
class SmartMergeMax;

// Enum to specify test type
enum class TestType {
    COMPETITIVE,  // Win vs Loss comparison
    MOVE_COUNT,   // Number of moves per game
    TIME_PER_MOVE // Time taken per move
};

// Structure to store test results for a single game
struct GameTestResult {
    int gameNumber;
    int startNumber;
    int gridSize;
    GameOverReason algo1Reason;
    GameOverReason algo2Reason;
    int algo1MoveCount;
    int algo2MoveCount;
    double algo1TotalTime;  // In milliseconds
    double algo2TotalTime;  // In milliseconds
    bool algo1Win;
    bool algo2Win;
    std::string winnerName;
};

// Structure for summary statistics
struct TestSummary {
    int gamesPlayed;
    int algo1Wins;
    int algo2Wins;
    int draws;
    double algo1WinRate;
    double algo2WinRate;
    double avgAlgo1Moves;
    double avgAlgo2Moves;
    double avgAlgo1TimePerMove; // In milliseconds
    double avgAlgo2TimePerMove; // In milliseconds
};

// Class for algorithm testing
class AlgoTester {
public:
    // Constructor
    AlgoTester(const std::string& algo1Name = "SmartMergeMax",
               const std::string& algo2Name = "ExpectimaxAI");

    // Destructor
    ~AlgoTester();

    // Run a batch of tests with specified parameters
    std::vector<GameTestResult> runTests(
        int numGames,
        int startNumber,
        int gridSize,
        TestType testType = TestType::COMPETITIVE
    );

    // Run tests with multiple configurations
    std::map<std::tuple<int, int>, std::vector<GameTestResult>> runMultipleTests(
        int numGames,
        const std::vector<int>& startNumbers,
        const std::vector<int>& gridSizes,
        TestType testType = TestType::COMPETITIVE
    );

    // Generate summary statistics from results
    TestSummary generateSummary(const std::vector<GameTestResult>& results);

    // Generate CSV file with detailed results
    void exportResultsToCSV(const std::vector<GameTestResult>& results, const std::string& filename);

    // Generate CSV file with summary statistics
    void exportSummaryToCSV(const std::map<std::tuple<int, int>, TestSummary>& summaries,
                           const std::string& filename);

    // Print results to console
    void printResults(const std::vector<GameTestResult>& results);

    // Print summary to console
    void printSummary(const TestSummary& summary);
      std::string testTypeToString(TestType testType);

private:
    std::string algo1Name;
    std::string algo2Name;

    // Run a single test game and return detailed results
    GameTestResult runSingleTest(int gameNumber, int startNumber, int gridSize, TestType testType);

    // Helper method to get a string representation of TestType


    // Helper method to format a reason string
    std::string formatReasonString(GameOverReason reason, int moveCount);
};

#endif // ALGO_TESTER_H
