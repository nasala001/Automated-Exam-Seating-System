#include "InputValidator.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// Helper to trim leading/trailing whitespace
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Helper to check if string contains only digits
static bool isDigits(const std::string& str) {
    if (str.empty()) return false;
    return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isdigit(c); });
}

namespace shresh {

// Split row by comma, maintaining simple RFC-4180 CSV structure
std::vector<std::string> InputValidator::splitCSV(const std::string& rawRow) const {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    for (size_t i = 0; i < rawRow.size(); ++i) {
        char c = rawRow[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            tokens.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    tokens.push_back(trim(current));
    return tokens;
}

void InputValidator::reset() {
    seenRegistrations_.clear();
    seenRollNumbers_.clear();
    lastError_ = "";
}

bool InputValidator::validateStudentData(const std::string& rawRow) {
    std::vector<std::string> tokens = splitCSV(rawRow);
    
    // Check fields count (Name, RegistrationNo, RollNo, Program, [Section], Department, Subject, Semester, IsImpaired)
    if (tokens.size() != 8 && tokens.size() != 9) {
        lastError_ = "Improper student formatting: Expected 8 or 9 comma-separated fields, but parsed " + 
                     std::to_string(tokens.size()) + ". Row: \"" + rawRow + "\"";
        return false;
    }

    std::string name = tokens[0];
    std::string registrationNo = tokens[1];
    std::string rollNo = tokens[2];
    std::string program = tokens[3];
    std::string section;
    std::string department;
    std::string subject;
    std::string semesterStr;
    std::string impairedStr;

    if (tokens.size() == 8) {
        section = "";
        department = tokens[4];
        subject = tokens[5];
        semesterStr = tokens[6];
        impairedStr = tokens[7];
    } else {
        section = tokens[4];
        department = tokens[5];
        subject = tokens[6];
        semesterStr = tokens[7];
        impairedStr = tokens[8];
    }

    // 1. Check for empty fields (except section which can be empty)
    if (name.empty() || rollNo.empty() || registrationNo.empty() || 
        program.empty() || department.empty() || subject.empty() || 
        semesterStr.empty() || impairedStr.empty()) {
        lastError_ = "Empty fields detected in student data. Name: '" + name + "', Roll No: '" + rollNo + "'";
        return false;
    }

    // 2. Validate boolean format (0 or 1, or case-insensitive "true"/"false")
    auto validateBool = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
        return (s == "0" || s == "1" || s == "true" || s == "false");
    };

    if (!validateBool(impairedStr)) {
        lastError_ = "Invalid boolean flag for student '" + name + "'. Expected 0/1 or true/false, got: Impaired='" + impairedStr + "'";
        return false;
    }

    // 3. Roll Number validation: ensure it doesn't contain weird characters
    if (rollNo.find_first_of("%;&@$") != std::string::npos) {
        lastError_ = "Malformed roll number: '" + rollNo + "' contains invalid characters.";
        return false;
    }

    // 4. Registration Number check (minimum structure, e.g. must not contain invalid symbols)
    if (registrationNo.find_first_of("%;&@$") != std::string::npos) {
        lastError_ = "Malformed registration number: '" + registrationNo + "' contains invalid characters.";
        return false;
    }

    // 5. Unique Registration Number Check (across the parsed file)
    if (!seenRegistrations_.insert(registrationNo).second) {
        lastError_ = "Duplicate registration number detected: '" + registrationNo + "' for student '" + name + "'";
        return false;
    }

    // 6. Unique Roll Number Check (across the parsed file)
    if (!seenRollNumbers_.insert(rollNo).second) {
        lastError_ = "Duplicate roll number detected: '" + rollNo + "' for student '" + name + "'";
        return false;
    }

    return true;
}


bool InputValidator::validateRoomData(const std::string& rawRow) {
    std::vector<std::string> tokens = splitCSV(rawRow);

    // Expected format either:
    // 5 fields: RoomCode, BlockCode, Capacity, Rows, Columns
    // 9 fields: RoomCode, BlockCode, Capacity, Rows, Columns, RoomType, BenchRows, BenchCols, SeatsPerBench
    if (tokens.size() != 5 && tokens.size() != 9) {
        lastError_ = "Improper room formatting: Expected 5 or 9 comma-separated fields, but parsed " + 
                     std::to_string(tokens.size()) + ". Row: \"" + rawRow + "\"";
        return false;
    }

    std::string roomCode = tokens[0];
    std::string blockCode = tokens[1];
    std::string capacityStr = tokens[2];
    std::string rowsStr = tokens[3];
    std::string columnsStr = tokens[4];

    // Check basic non-empty
    if (roomCode.empty() || blockCode.empty() || capacityStr.empty() || 
        rowsStr.empty() || columnsStr.empty()) {
        lastError_ = "Empty fields detected in room data. Room Code: '" + roomCode + "'";
        return false;
    }

    // Verify basic dimensions are numbers
    if (!isDigits(capacityStr) || !isDigits(rowsStr) || !isDigits(columnsStr)) {
        lastError_ = "Non-numeric values in room dimensions: Capacity='" + capacityStr + 
                     "', Rows='" + rowsStr + "', Columns='" + columnsStr + "'";
        return false;
    }

    int capacity = std::stoi(capacityStr);
    int rows = std::stoi(rowsStr);
    int columns = std::stoi(columnsStr);

    std::string roomType = "Hall";
    int benchRows = rows;
    int benchCols = columns;
    int seatsPerBench = 1;

    if (tokens.size() == 9) {
        roomType = tokens[5];
        std::string bRowsStr = tokens[6];
        std::string bColsStr = tokens[7];
        std::string sPerBenchStr = tokens[8];

        if (roomType.empty() || bRowsStr.empty() || bColsStr.empty() || sPerBenchStr.empty()) {
            lastError_ = "Empty values in room type or bench details. Room Code: '" + roomCode + "'";
            return false;
        }

        if (!isDigits(bRowsStr) || !isDigits(bColsStr) || !isDigits(sPerBenchStr)) {
            lastError_ = "Non-numeric values in bench layout: BenchRows='" + bRowsStr + 
                         "', BenchCols='" + bColsStr + "', SeatsPerBench='" + sPerBenchStr + "'";
            return false;
        }

        benchRows = std::stoi(bRowsStr);
        benchCols = std::stoi(bColsStr);
        seatsPerBench = std::stoi(sPerBenchStr);

        // Standardize room type checking
        std::string lowerType = roomType;
        std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), [](unsigned char c) { return std::tolower(c); });
        if (lowerType != "hall" && lowerType != "classroom") {
            lastError_ = "Invalid Room Type: '" + roomType + "'. Must be 'Hall' or 'Classroom'.";
            return false;
        }
    }

    // Positivity constraints
    if (capacity <= 0) {
        lastError_ = "Invalid Room Capacity: capacity " + std::to_string(capacity) + " must be greater than 0.";
        return false;
    }
    if (rows <= 0 || columns <= 0) {
        lastError_ = "Invalid Room Grid: dimensions Rows=" + std::to_string(rows) + 
                     ", Cols=" + std::to_string(columns) + " must both be greater than 0.";
        return false;
    }
    if (benchRows <= 0 || benchCols <= 0 || seatsPerBench <= 0) {
        lastError_ = "Invalid Bench layout parameters: all bench layout dimensions must be greater than 0.";
        return false;
    }

    // Logical structural check: grid rows x cols must be equal or greater than room capacity
    int totalSeats = rows * columns;
    if (totalSeats < capacity) {
        lastError_ = "Logical Layout Violation for Room " + roomCode + ": Grid size (" + std::to_string(rows) + " x " + 
                     std::to_string(columns) + " = " + std::to_string(totalSeats) + " seats) is smaller than specified capacity (" + 
                     std::to_string(capacity) + ").";
        return false;
    }

    // Verify bench layout matches grid dimensions: benchRows * benchCols * seatsPerBench must match or exceed capacity
    int totalBenchSeats = benchRows * benchCols * seatsPerBench;
    if (totalBenchSeats < capacity) {
        lastError_ = "Logical Layout Violation for Room " + roomCode + ": Bench layout capacity (" + 
                     std::to_string(benchRows) + " x " + std::to_string(benchCols) + " x " + std::to_string(seatsPerBench) + 
                     " = " + std::to_string(totalBenchSeats) + " seats) is smaller than specified capacity (" + 
                     std::to_string(capacity) + ").";
        return false;
    }

    return true;
}

} // namespace shresh
