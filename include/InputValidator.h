#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

#include "DataStructures.h"
#include <string>
#include <vector>
#include <unordered_set>

class InputValidator {
private:
    std::unordered_set<std::string> seenRegistrations_;
    std::unordered_set<std::string> seenRollNumbers_;
    std::string lastError_;

    // Helper to split a CSV string, taking care of commas
    std::vector<std::string> splitCSV(const std::string& rawRow) const;

public:
    InputValidator() = default;

    // Clears seen registries and roll numbers to start a fresh file import validation
    void reset();

    // Validates a raw CSV/TXT line representing Student data
    // Expected format: Name, RollNo, RegNo, Program, Department, Subject, HasContagious(0/1), IsImpaired(0/1)
    bool validateStudentData(const std::string& rawRow);

    // Validates a raw CSV/TXT line representing Room data
    // Expected format: RoomCode, BlockCode, Capacity, Rows, Columns
    bool validateRoomData(const std::string& rawRow);

    // Helper to retrieve the last validation error message
    std::string getLastError() const { return lastError_; }
};

#endif // INPUT_VALIDATOR_H
