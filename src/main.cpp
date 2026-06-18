#include "DataStructures.h"
#include "RuleValidator.h"
#include "InputValidator.h"
#include "ConcreteRules.h"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>

// Helper to print section titles
void printHeader(const std::string& title) {
    std::cout << "\n======================================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "======================================================================\n";
}

// Helper to print a test report
void printReport(const ValidationReport& report) {
    if (report.isValid) {
        std::cout << "  [PASSED] Seating Plan is Valid (No hard errors).\n";
    } else {
        std::cout << "  [FAILED] Seating Plan is Invalid (Hard errors found).\n";
    }

    if (!report.hardErrors.empty()) {
        std::cout << "  --- HARD ERRORS ---\n";
        for (const auto& err : report.hardErrors) {
            std::cout << "  * " << err << "\n";
        }
    }

    if (!report.softWarnings.empty()) {
        std::cout << "  --- SOFT WARNINGS (Preferences/Overrideable) ---\n";
        for (const auto& wrn : report.softWarnings) {
            std::cout << "  * " << wrn << "\n";
        }
    }
}

// Test function for InputValidator
void testInputValidator() {
    printHeader("1. TESTING INPUT VALIDATOR");
    InputValidator csvValidator;

    // Test valid student data
    std::string validStudent = "Aayush Bhatta, CE-05, 024412-19, CE, DoCSE, COMP102, 0, 0";
    std::cout << "Valid Student CSV: \"" << validStudent << "\"\n";
    if (csvValidator.validateStudentData(validStudent)) {
        std::cout << "  -> Validated successfully!\n";
    } else {
        std::cout << "  -> Failed: " << csvValidator.getLastError() << "\n";
    }

    // Test invalid student (duplicate registration)
    std::string duplicateStudent = "Niranjan Gupta, CE-06, 024412-19, CE, DoCSE, COMP102, 0, 0";
    std::cout << "\nDuplicate Student Reg CSV: \"" << duplicateStudent << "\"\n";
    if (csvValidator.validateStudentData(duplicateStudent)) {
        std::cout << "  -> Validated successfully (unexpected)!\n";
    } else {
        std::cout << "  -> Failed (expected): " << csvValidator.getLastError() << "\n";
    }

    // Test malformed roll number
    std::string malformedStudent = "Rohan Shrestha, CE-&07, 024415-19, CE, DoCSE, COMP102, 0, 0";
    std::cout << "\nMalformed Roll No CSV: \"" << malformedStudent << "\"\n";
    if (csvValidator.validateStudentData(malformedStudent)) {
        std::cout << "  -> Validated successfully (unexpected)!\n";
    } else {
        std::cout << "  -> Failed (expected): " << csvValidator.getLastError() << "\n";
    }

    // Test valid room data
    std::string validRoom = "Block-C-201, C, 30, 6, 5";
    std::cout << "\nValid Room CSV: \"" << validRoom << "\"\n";
    if (csvValidator.validateRoomData(validRoom)) {
        std::cout << "  -> Validated successfully!\n";
    } else {
        std::cout << "  -> Failed: " << csvValidator.getLastError() << "\n";
    }

    // Test invalid room capacity layout mismatch
    std::string invalidRoom = "Block-C-202, C, 40, 5, 5";
    std::cout << "\nInvalid Room CSV (Grid size 25 < capacity 40): \"" << invalidRoom << "\"\n";
    if (csvValidator.validateRoomData(invalidRoom)) {
        std::cout << "  -> Validated successfully (unexpected)!\n";
    } else {
        std::cout << "  -> Failed (expected): " << csvValidator.getLastError() << "\n";
    }
}

// Test function for Rules
void testSeatingRules() {
    printHeader("2. TESTING RULE VALIDATION ENGINES");

    // Initialize Room & Setup SeatPlan structure (6 rows, 6 columns, capacity 30)
    Room room{"Block-A-101", "A", 30, 6, 6, 4, "Hall", 6, 6, 1};
    SeatPlan plan;
    plan.room = room;
    plan.grid.resize(room.rows, std::vector<std::optional<Student>>(room.columns, std::nullopt));

    // Sample Students
    Student s1{"Susan", "CE-01", "024501-19", "CE", "DoCSE", "COMP102", false, false};
    Student s2{"Ashish", "CE-02", "024502-19", "CE", "DoCSE", "COMP102", false, false};
    Student s3{"Bipul", "AI-01", "024503-19", "AI", "DoCSE", "COMP102", false, false};
    Student s4{"Binita", "CE-03", "024504-19", "CE", "DoCSE", "MATH104", false, false};
    Student s5{"Dinesh", "CE-04", "024505-19", "CE", "DoCSE", "COMP102", true, false}; // Infected
    Student s6{"Elina", "ME-01", "024506-19", "ME", "DoME", "MATH104", false, true};    // Physically Impaired

    // ----------------------------------------------------
    // Scenario A: Standard Valid Seating Arrangement
    // ----------------------------------------------------
    std::cout << "\n--- Scenario A: Valid Arrangement ---\n";
    // We space out student s1 (CE) and s3 (AI) horizontally, placing impaired s6 in row 0
    plan.grid[0][0] = s6; // Impaired in front row (Row 0)
    plan.grid[1][1] = s1; // CE student
    plan.grid[1][3] = s3; // AI student (separated from CE, different program, same department is allowed)
    plan.grid[2][1] = s4; // CE student in a different row (vertically behind s1 - preferred program alignment)

    RuleValidator validator;
    ValidationReport reportA = validator.validate(plan);
    printReport(reportA);

    // ----------------------------------------------------
    // Scenario B: Violating Horizontal Separation, Consecutive Rolls, and Subject Rules
    // ----------------------------------------------------
    std::cout << "\n--- Scenario B: Multiple Horizontal Failures ---\n";
    // Place s1 (CE, Roll 1, COMP102) right next to s2 (CE, Roll 2, COMP102)
    plan.grid[3][0] = s1;
    plan.grid[3][1] = s2;

    ValidationReport reportB = validator.validate(plan);
    printReport(reportB);

    // Reset row 3
    plan.grid[3][0] = std::nullopt;
    plan.grid[3][1] = std::nullopt;

    // ----------------------------------------------------
    // Scenario C: Contagious Disease and Accessibility Failures
    // ----------------------------------------------------
    std::cout << "\n--- Scenario C: Infection & Accessibility Rule Failures ---\n";
    // Place infected student s5 in the middle of standard student s1
    plan.grid[1][1] = s1;
    plan.grid[1][2] = s5; // Infected right next to s1

    // Move impaired student s6 to row 3 (should be row 0)
    plan.grid[0][0] = std::nullopt;
    plan.grid[3][3] = s6;

    ValidationReport reportC = validator.validate(plan);
    printReport(reportC);

    // ----------------------------------------------------
    // Scenario D: Invigilator and Capacity Violations
    // ----------------------------------------------------
    std::cout << "\n--- Scenario D: Invigilator & Capacity Checks ---\n";
    // Reset plan
    for (auto& row : plan.grid) std::fill(row.begin(), row.end(), std::nullopt);
    
    // Low invigilator room details (only 1 invigilator)
    plan.room.numInvigilators = 1;
    plan.room.capacity = 2; // Very small capacity

    // Place 3 students (capacity is 2, and amanuensis for s6 would add +1)
    plan.grid[0][0] = s6; // physically impaired (needs 1 assistant)
    plan.grid[0][2] = s1;
    plan.grid[0][4] = s2;

    ValidationReport reportD = validator.validate(plan);
    printReport(reportD);
}

// ----------------------------------------------------
// 3. Interactive Room Layout & Seating Validator
// ----------------------------------------------------
void runInteractiveMode() {
    printHeader("3. INTERACTIVE ROOM LAYOUT & SEATING VALIDATOR");
    
    std::string roomCode, blockCode;
    int roomTypeOption = 1;
    int capacity = 0;
    int numStudents = 0;

    std::cout << "Enter Room/Hall Code (e.g. Block-B-201): ";
    std::cin >> roomCode;
    std::cout << "Enter Block Code (e.g. B): ";
    std::cin >> blockCode;
    
    std::cout << "Select Room Type:\n  1. Hall (Individual chairs/tables grid)\n  2. Classroom (Benches layout)\nEnter choice (1 or 2): ";
    std::cin >> roomTypeOption;

    Room room;
    room.roomCode = roomCode;
    room.blockCode = blockCode;

    if (roomTypeOption == 2) {
        room.roomType = "Classroom";
        std::cout << "Enter number of Bench Rows: ";
        std::cin >> room.benchRows;
        std::cout << "Enter number of Bench Columns: ";
        std::cin >> room.benchCols;
        std::cout << "Enter number of Seats per Bench (e.g. 2): ";
        std::cin >> room.seatsPerBench;
        
        room.rows = room.benchRows;
        room.columns = room.benchCols * room.seatsPerBench;
        
        std::cout << "Enter Room Capacity (or 0 to auto-calculate): ";
        std::cin >> capacity;
        if (capacity <= 0) {
            room.capacity = room.benchRows * room.benchCols * room.seatsPerBench;
        } else {
            room.capacity = capacity;
        }
    } else {
        room.roomType = "Hall";
        std::cout << "Enter number of Grid Rows: ";
        std::cin >> room.rows;
        std::cout << "Enter number of Grid Columns: ";
        std::cin >> room.columns;
        
        room.benchRows = room.rows;
        room.benchCols = room.columns;
        room.seatsPerBench = 1;
        
        std::cout << "Enter Room Capacity (or 0 to auto-calculate): ";
        std::cin >> capacity;
        if (capacity <= 0) {
            room.capacity = room.rows * room.columns;
        } else {
            room.capacity = capacity;
        }
    }

    std::cout << "Enter number of Students to seat: ";
    std::cin >> numStudents;
    std::cout << "Enter number of Invigilators assigned: ";
    std::cin >> room.numInvigilators;

    std::cout << "\nGenerating Mock Student Seating...\n";
    SeatPlan plan;
    plan.room = room;
    plan.grid.resize(room.rows, std::vector<std::optional<Student>>(room.columns, std::nullopt));

    int studentMode = 1;
    std::cout << "Select student arrangement style:\n  1. Alternating Programs (CE / AI / ME) - reduces cheating violations\n  2. Homogeneous Programs (CE only) - triggers Same Program violations\nEnter choice (1 or 2): ";
    std::cin >> studentMode;

    char addInfected = 'n', addImpaired = 'n';
    std::cout << "Add 1 contagious student to the middle of the grid? (y/n): ";
    std::cin >> addInfected;
    std::cout << "Add 1 physically impaired student? (y/n): ";
    std::cin >> addImpaired;

    // Create a vector of mock students
    std::vector<Student> studentsList;
    for (int i = 0; i < numStudents; ++i) {
        std::string roll = std::to_string(100 + i);
        std::string reg = "0245" + std::to_string(10 + i) + "-20";
        std::string prog = "CE";
        std::string dept = "DoCSE";
        std::string subj = "COMP102";

        if (studentMode == 1) {
            int cycle = i % 3;
            if (cycle == 0) {
                prog = "CE";
                dept = "DoCSE";
                subj = "COMP102";
            } else if (cycle == 1) {
                prog = "AI";
                dept = "DoCSE";
                subj = "COMP102";
            } else {
                prog = "ME";
                dept = "DoME";
                subj = "MATH104";
            }
        }
        
        studentsList.push_back(Student{"Student_" + std::to_string(i + 1), roll, reg, prog, dept, subj, false, false});
    }

    // Adjust special states if requested
    if (addInfected == 'y' || addInfected == 'Y') {
        if (!studentsList.empty()) {
            studentsList[studentsList.size() / 2].hasContagiousDisease = true;
            studentsList[studentsList.size() / 2].name += " (INFECTED)";
        }
    }
    if (addImpaired == 'y' || addImpaired == 'Y') {
        if (!studentsList.empty()) {
            studentsList[0].isPhysicallyImpaired = true;
            studentsList[0].name += " (IMPAIRED)";
        }
    }

    // Seat students sequentially in grid
    int sIdx = 0;
    int totalCapacity = room.rows * room.columns;
    int studentsToSeat = std::min(numStudents, totalCapacity);

    for (int r = 0; r < room.rows && sIdx < studentsToSeat; ++r) {
        for (int c = 0; c < room.columns && sIdx < studentsToSeat; ++c) {
            plan.grid[r][c] = studentsList[sIdx++];
        }
    }

    // Visual Display of Room Layout
    std::cout << "\n------------------------------------------------------------\n";
    std::cout << " VISUAL SEATING LAYOUT MAP (" << room.roomType << ")\n";
    std::cout << "------------------------------------------------------------\n";
    if (room.roomType == "Classroom") {
        std::cout << "  (Double brackets [ ] represent one bench of " << room.seatsPerBench << " seats)\n\n";
    }

    for (int r = 0; r < room.rows; ++r) {
        std::cout << "Row " << std::setw(2) << r << ":  ";
        for (int c = 0; c < room.columns; ++c) {
            if (room.roomType == "Classroom" && c % room.seatsPerBench == 0) {
                std::cout << "[";
            }

            const auto& seat = plan.grid[r][c];
            if (seat.has_value()) {
                std::cout << std::setw(3) << seat->program << "-" << std::setw(3) << seat->rollNo;
            } else {
                std::cout << " ------- ";
            }

            if (room.roomType == "Classroom" && (c + 1) % room.seatsPerBench == 0) {
                std::cout << "] ";
            } else {
                std::cout << " ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "------------------------------------------------------------\n";

    // Run verification
    RuleValidator validator;
    ValidationReport report = validator.validate(plan);
    printReport(report);
}

int main() {
    std::cout << "======================================================================\n";
    std::cout << " AUTOMATED EXAM SEATING MANAGEMENT SYSTEM (AESMS) - VALIDATION SUITE\n";
    std::cout << "======================================================================\n";

    int choice = 1;
    std::cout << "Select running mode:\n";
    std::cout << "  1. Run Automated Rule Validation Tests (Scenario A, B, C, D)\n";
    std::cout << "  2. Interactive Room & Seating Validator\n";
    std::cout << "Enter choice (1 or 2): ";
    std::cin >> choice;

    if (choice == 2) {
        runInteractiveMode();
    } else {
        testInputValidator();
        testSeatingRules();
    }

    return 0;
}
