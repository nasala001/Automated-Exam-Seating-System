#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <string>
#include <vector>
#include <memory>

namespace shresh {

struct Student {
    std::string name;
    std::string rollNo;          // e.g., "101", "CE-05"
    std::string registrationNo;  // e.g., "024512-19"
    std::string program;         // e.g., "CE", "AI", "ME"
    std::string section;         // e.g., "A", "B"
    std::string department;      // e.g., "DoCSE", "DoME"
    std::string subject;         // e.g., "COMP102", "MATH104"
    int semester = 1;
    bool hasContagiousDisease = false;
    bool isPhysicallyImpaired = false;
};

struct Room {
    std::string roomCode;        // e.g., "Block-A-201"
    std::string blockCode;       // e.g., "A"
    int capacity = 0;
    int rows = 0;                // Total layout rows of seats
    int columns = 0;             // Total layout columns of seats
    int numInvigilators = 0;     // Assigned invigilators for rule checks
    std::string roomType;        // "Hall" or "Classroom"
    int benchRows = 0;           // Number of bench rows (if Classroom)
    int benchCols = 0;           // Number of bench columns (if Classroom)
    int seatsPerBench = 2;       // Standard seats per bench (usually 2 in classrooms)
};

struct SeatPlan {
    Room room;
    // 2D representation of seats where grid[row][col] is std::optional<Student>
    // Row and Column sizes are defined by Room.rows and Room.columns
    std::vector<std::vector<std::shared_ptr<Student>>> grid;
};

} // namespace shresh


#endif // DATA_STRUCTURES_H
