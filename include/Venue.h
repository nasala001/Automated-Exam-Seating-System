#pragma once

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Section
// Represents one named section within a large hall (e.g. "A" with 3 columns).
// ---------------------------------------------------------------------------
struct Section {
    std::string label; // e.g. "A", "B"
    int         cols;  // number of columns in this section
};

// ---------------------------------------------------------------------------
// Seat
// Represents a single physical seat inside a room or hall.
// ---------------------------------------------------------------------------
struct Seat {
    int         row;          // 1-indexed row position
    int         col;          // 1-indexed column position (within the full grid)
    int         seatNumber;   // sequential seat number across the grid (1-based)
    std::string seatCode;     // e.g. "B8-304-12" or "MPH-A1-5"
    int         studentIndex; // index into a student list (-1 = unoccupied)

    Seat() : row(0), col(0), seatNumber(0), studentIndex(-1) {}
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

    // Returns the display label for a column (0-indexed).
    // Base returns "Col 1", "Col 2", etc.
    virtual std::string columnLabel(int colIndex) const;

    // Pretty-prints the seat grid to stdout.
    void printLayout() const;

    // Returns total seat count.
    int totalSeats() const { return rows * cols; }

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
// LargeHall  — multipurpose hall divided into lettered sections
// Seat code format:  HALLCODE-SECTION{col}-{row}  (e.g. "MPH-A1-5")
//
// A hall can have multiple sections. Each section has its own label
// and column count. Rows are shared across all sections.
//   Section A (3 cols) + Section B (4 cols) → 7 total columns
//   Column labels: A1 A2 A3 B1 B2 B3 B4
// ---------------------------------------------------------------------------
class LargeHall : public Venue {
public:
    std::vector<Section> sections;

    // rows is shared across all sections.
    // Total cols = sum of all section column counts.
    LargeHall(const std::string& hallName,
              const std::vector<Section>& sections,
              int rows);

    void generateLayout() override;
    std::string columnLabel(int colIndex) const override;

private:
    // Maps a flat column index (0-based) to section label + column-within-section.
    // e.g. colIndex=4 with sections A(3 cols), B(4 cols) → ("B", 2)
    std::pair<std::string, int> resolveColumn(int colIndex) const;
};
