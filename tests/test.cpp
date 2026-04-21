#include "test.h"
#include <iostream>
#include <vector>
#include <ctime>

static std::vector<TestResult> results;

void runTest(TestResult result) {
    results.push_back(result);
    std::cout << "  " << result.testName << ": " << result.message;
    if (result.passed) {
        std::cout << " OK";
    } else {
        std::cout << " FAILED";
    }
    std::cout << std::endl;
}

int runAllTests() {
    std::cout << "\n========================================" << std::endl;
    int passed = 0;
    for (const auto& r : results) {
        if (r.passed) passed++;
    }
    std::cout << "Results: " << passed << "/" << results.size() << " tests passed" << std::endl;
    std::cout << "========================================\n" << std::endl;
    return results.size() - passed;
}
