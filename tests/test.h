#pragma once
#include <string>

struct TestResult {
    std::string testName;
    bool passed;
    std::string message;
    
    static TestResult pass(const std::string& name) {
        return {name, true, "OK"};
    }
    static TestResult fail(const std::string& name, const std::string& msg) {
        return {name, false, msg};
    }
};

void runTest(TestResult result);
int runAllTests();

#define TEST_PASS(name, condition) \
    do { \
        if (condition) { \
            runTest(TestResult::pass(name)); \
        } else { \
            runTest(TestResult::fail(name, "Assertion failed: " #condition)); \
        } \
    } while(0)
