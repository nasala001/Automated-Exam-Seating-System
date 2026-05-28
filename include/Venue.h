#pragma once

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Seat
// Represents a single physical seat inside a room or hall.
// ---------------------------------------------------------------------------
struct Seat {
    int         row;        // 1-indexed row position
    int         col;        // 1-indexed column position
    int         seatNumber; // sequential seat number across the grid (1-based)
    std::string seatCode;   // final generated code, e.g. "B8-304-12" or "MPH-D-5"
};

// ---------------------------------------------------------------------------
// Venue  (abstract base class)
// Holds the 2-D layout of seats and owns code-generation logic.
// ---------------------------------------------------------------------------
class Venue {
public:
    std::string name;  // full human-readable name, e.g. "Block 8"
    std::string code;  // short normalized code, e.g. "B8"
    int         rows;
    int         cols;
    std::vector<std::vector<Seat>> layout; // layout[row][col]

    Venue(const std::string& fullName, int rows, int cols);
    virtual ~Venue() = default;

    // Builds the 2-D layout and assigns seat codes.
    // Must be implemented by each derived class.
    virtual void generateLayout() = 0;

    // Pretty-prints the seat grid to stdout.
    void printLayout() const;

protected:
    // Fills layout with Seat objects (seatCode left empty).
    void initGrid();
};

// ---------------------------------------------------------------------------
// ExamRoom  — standard classroom / block room
// Seat code format:  BLOCKCODE-ROOMNO-SEATNO   (e.g. "B8-304-12")
// ---------------------------------------------------------------------------
class ExamRoom : public Venue {
public:
    std::string roomNumber; // e.g. "304"

    ExamRoom(const std::string& blockName,
             const std::string& roomNumber,
             int rows, int cols);

    void generateLayout() override;
};

// ---------------------------------------------------------------------------
// LargeHall  — multipurpose hall divided into lettered groups
// Seat code format:  HALLCODE-GROUP-SEATNO     (e.g. "MPH-D-21")
// ---------------------------------------------------------------------------
class LargeHall : public Venue {
public:
    std::string group; // e.g. "D"

    LargeHall(const std::string& hallName,
              const std::string& group,
              int rows, int cols);

    void generateLayout() override;
};
