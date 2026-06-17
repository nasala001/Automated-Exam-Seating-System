#pragma once

#include "Student.h"
#include "Venue.h"

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// SeatingEngine
// Assigns students to seats in a venue while enforcing examination rules:
//   Rule 1  — Do not exceed room capacity
//   Rule 2  — One student per seat, one seat per student
//   Rule 5  — Same program not side-by-side horizontally
//   Rule 6  — Same semester not side-by-side horizontally
//   Rule 7  — Consecutive roll numbers not side-by-side horizontally
//   Rule 8  — Same program prefers vertical (same column) arrangement
//   Rule 10 — Same subject not side-by-side horizontally
//   Rule 11 — Distribute programs across columns
// ---------------------------------------------------------------------------
class SeatingEngine {
public:
    // Assigns students to seats in the venue.
    // Returns true if all students were placed (possibly with soft warnings).
    // Returns false if a hard failure occurred (e.g. more students than seats).
    bool assignSeats(Venue& venue, std::vector<Student>& students);

    // Prints a formatted seating plan table to stdout.
    void printSeatingPlan(const Venue& venue,
                          const std::vector<Student>& students) const;

    // Returns any warnings generated during the last assignment.
    const std::vector<std::string>& getWarnings() const { return warnings; }

private:
    std::vector<std::string> warnings;

    // --- Rule-checking helpers -----------------------------------------------

    // Returns true if placing studentIdx at (row, col) satisfies horizontal
    // separation rules with already-placed neighbors.
    bool isValidPlacement(const Venue& venue,
                          const std::vector<Student>& students,
                          int row, int col,
                          int studentIdx) const;

    // Checks a single horizontal neighbor pair for violations.
    // Returns true if no violation exists.
    bool checkNeighbor(const std::vector<Student>& students,
                       int studentA, int studentB) const;

    // Attempts to fix remaining violations by swapping seats.
    void repairViolations(Venue& venue,
                          const std::vector<Student>& students);

    // Counts remaining horizontal violations in the layout.
    int countViolations(const Venue& venue,
                        const std::vector<Student>& students) const;
};
