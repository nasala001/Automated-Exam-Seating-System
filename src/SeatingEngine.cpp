#include "SeatingEngine.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

using namespace std;

// ---------------------------------------------------------------------------
// Helper: extract the numeric suffix from a roll number for consecutive check.
// e.g. "CE-2081-003" → 3,  "2081-2-03-015" → 15
// Takes the last group of digits in the string.
// ---------------------------------------------------------------------------
static int rollSuffix(const string &roll) {
  int num = -1;
  string digits;
  for (int i = static_cast<int>(roll.size()) - 1; i >= 0; --i) {
    if (isdigit(static_cast<unsigned char>(roll[i]))) {
      digits = roll[i] + digits;
    } else if (!digits.empty()) {
      break; // stop at the first non-digit after we've collected some
    }
  }
  if (!digits.empty()) {
    try {
      num = stoi(digits);
    } catch (...) {
    }
  }
  return num;
}

// ---------------------------------------------------------------------------
// checkNeighbor — checks one horizontal pair for rule violations
// ---------------------------------------------------------------------------
bool SeatingEngine::checkNeighbor(const vector<Student> &students,
                                  int idxA, int idxB) const {
  if (idxA < 0 || idxB < 0)
    return true; // one seat is empty — no violation

  const Student &a = students[idxA];
  const Student &b = students[idxB];

  // Rule 5: Same program not side-by-side horizontally
  if (a.program == b.program)
    return false;

  // Rule 6: Same semester not side-by-side horizontally
  if (a.semester == b.semester && a.semester != 0)
    return false;

  // Rule 7: Consecutive roll numbers not side-by-side horizontally
  int ra = rollSuffix(a.examRollNo);
  int rb = rollSuffix(b.examRollNo);
  if (ra >= 0 && rb >= 0 && abs(ra - rb) == 1)
    return false;

  // Rule 10: Same subject not side-by-side horizontally
  if (a.subject == b.subject && !a.subject.empty())
    return false;

  return true; // no violations
}

// ---------------------------------------------------------------------------
// isValidPlacement — checks if placing studentIdx at (row, col) is OK
// ---------------------------------------------------------------------------
bool SeatingEngine::isValidPlacement(const Venue &venue,
                                     const vector<Student> &students,
                                     int row, int col,
                                     int studentIdx) const {
  // Check left neighbor
  if (col > 0) {
    int leftIdx = venue.layout[row][col - 1].studentIndex;
    if (!checkNeighbor(students, studentIdx, leftIdx))
      return false;
  }
  // Check right neighbor
  if (col < venue.cols - 1) {
    int rightIdx = venue.layout[row][col + 1].studentIndex;
    if (!checkNeighbor(students, studentIdx, rightIdx))
      return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// countViolations — counts all horizontal neighbor violations
// ---------------------------------------------------------------------------
int SeatingEngine::countViolations(const Venue &venue,
                                   const vector<Student> &students) const {
  int violations = 0;
  for (int r = 0; r < venue.rows; ++r) {
    for (int c = 0; c < venue.cols - 1; ++c) {
      int idxA = venue.layout[r][c].studentIndex;
      int idxB = venue.layout[r][c + 1].studentIndex;
      if (idxA >= 0 && idxB >= 0) {
        if (!checkNeighbor(students, idxA, idxB))
          ++violations;
      }
    }
  }
  return violations;
}

// ---------------------------------------------------------------------------
// repairViolations — attempts to fix violations by swapping seats
// ---------------------------------------------------------------------------
void SeatingEngine::repairViolations(Venue &venue,
                                     const vector<Student> &students) {
  // Iterate multiple passes; stop when no improvement
  const int maxPasses = 50;
  for (int pass = 0; pass < maxPasses; ++pass) {
    int improved = 0;

    for (int r = 0; r < venue.rows; ++r) {
      for (int c = 0; c < venue.cols - 1; ++c) {
        int idxA = venue.layout[r][c].studentIndex;
        int idxB = venue.layout[r][c + 1].studentIndex;
        if (idxA < 0 || idxB < 0)
          continue;
        if (checkNeighbor(students, idxA, idxB))
          continue;

        // Found a violation at (r, c) and (r, c+1).
        // Try swapping seat (r, c) with every other occupied seat.
        int bestR = -1, bestC = -1;
        int currentViolations = countViolations(venue, students);

        for (int sr = 0; sr < venue.rows; ++sr) {
          for (int sc = 0; sc < venue.cols; ++sc) {
            if (sr == r && sc == c)
              continue;
            int other = venue.layout[sr][sc].studentIndex;
            if (other < 0)
              continue;

            // Trial swap
            swap(venue.layout[r][c].studentIndex,
                 venue.layout[sr][sc].studentIndex);
            int newViolations = countViolations(venue, students);

            if (newViolations < currentViolations) {
              bestR = sr;
              bestC = sc;
              currentViolations = newViolations;
            }

            // Undo trial swap
            swap(venue.layout[r][c].studentIndex,
                 venue.layout[sr][sc].studentIndex);
          }
        }

        if (bestR >= 0) {
          // Commit the best swap
          swap(venue.layout[r][c].studentIndex,
               venue.layout[bestR][bestC].studentIndex);
          ++improved;
        }
      }
    }

    if (improved == 0)
      break; // no more improvements possible
  }
}

// ---------------------------------------------------------------------------
// assignSeats — main assignment algorithm
// ---------------------------------------------------------------------------
bool SeatingEngine::assignSeats(Venue &venue, vector<Student> &students) {
  warnings.clear();

  int totalSeats = venue.totalSeats();
  int numStudents = static_cast<int>(students.size());

  // Rule 1: Capacity check
  if (numStudents > totalSeats) {
    warnings.push_back("WARNING: " + to_string(numStudents) +
                        " students exceed " + to_string(totalSeats) +
                        " available seats. Only the first " +
                        to_string(totalSeats) + " will be assigned.");
    numStudents = totalSeats;
  }

  if (numStudents == 0) {
    warnings.push_back("No students to assign.");
    return false;
  }

  // -----------------------------------------------------------------------
  // Step 1: Group students by program
  // -----------------------------------------------------------------------
  map<string, vector<int>> programGroups; // program → list of student indices
  for (int i = 0; i < numStudents; ++i) {
    programGroups[students[i].program].push_back(i);
  }

  // Sort each group by roll number (for consecutive roll separation)
  for (auto &entry : programGroups) {
    vector<int> &indices = entry.second;
    sort(indices.begin(), indices.end(), [&](int a, int b) {
      return students[a].examRollNo < students[b].examRollNo;
    });
  }

  // -----------------------------------------------------------------------
  // Step 2: Order programs by group size (largest first) for better spread
  // -----------------------------------------------------------------------
  vector<string> programOrder;
  for (const auto &entry : programGroups)
    programOrder.push_back(entry.first);
  sort(programOrder.begin(), programOrder.end(),
       [&](const string &a, const string &b) {
         return programGroups[a].size() > programGroups[b].size();
       });

  // -----------------------------------------------------------------------
  // Step 3: Assign programs to columns (Rule 11 — distribute across columns)
  //
  // Strategy: assign each program a "home column" in round-robin.
  // Fill each column top-to-bottom with students from the assigned program
  // (Rule 8 — same program prefers vertical arrangement).
  // -----------------------------------------------------------------------
  int numCols = venue.cols;
  int numRows = venue.rows;

  // Column → list of student indices to place there
  vector<vector<int>> columnAssignments(numCols);

  // Round-robin assignment: program_i → column (i % numCols)
  // If a program needs more rows than available, spill into the next
  // available column.
  vector<int> colFillCount(numCols, 0); // how many seats used per column

  for (const string &prog : programOrder) {
    const vector<int> &group = programGroups[prog];
    int placed = 0;

    // Find the best starting column for this program
    // (prefer columns with the fewest students so far)
    vector<int> colOrder(numCols);
    iota(colOrder.begin(), colOrder.end(), 0);
    sort(colOrder.begin(), colOrder.end(),
         [&](int a, int b) { return colFillCount[a] < colFillCount[b]; });

    for (int ci = 0; ci < numCols && placed < (int)group.size(); ++ci) {
      int col = colOrder[ci];
      while (colFillCount[col] < numRows &&
             placed < (int)group.size()) {
        columnAssignments[col].push_back(group[placed]);
        colFillCount[col]++;
        placed++;
      }
    }

    if (placed < (int)group.size()) {
      // Shouldn't happen if capacity check passed, but just in case
      warnings.push_back("Could not place all students from program " + prog);
    }
  }

  // -----------------------------------------------------------------------
  // Step 4: Place students into the grid (column-by-column, top-to-bottom)
  // -----------------------------------------------------------------------
  for (int c = 0; c < numCols; ++c) {
    for (int i = 0; i < (int)columnAssignments[c].size(); ++i) {
      venue.layout[i][c].studentIndex = columnAssignments[c][i];
    }
  }

  // -----------------------------------------------------------------------
  // Step 5: Validate and repair horizontal violations
  // -----------------------------------------------------------------------
  int violationsBefore = countViolations(venue, students);
  if (violationsBefore > 0) {
    repairViolations(venue, students);
    int violationsAfter = countViolations(venue, students);
    if (violationsAfter > 0) {
      warnings.push_back("INFO: " + to_string(violationsAfter) +
                          " horizontal separation rule violation(s) remain "
                          "after best-effort repair. This may be unavoidable "
                          "given the student/program distribution.");
    }
  }

  return true;
}

// ---------------------------------------------------------------------------
// printSeatingPlan — formatted output of student ↔ seat assignments
// ---------------------------------------------------------------------------
void SeatingEngine::printSeatingPlan(const Venue &venue,
                                     const vector<Student> &students) const {
  cout << "\n";
  cout << "================================================================"
          "==============================\n";
  cout << "  SEATING PLAN — " << venue.name << "  [" << venue.code << "]\n";
  cout << "================================================================"
          "==============================\n\n";

  // Table header
  cout << left << setw(14) << "Seat Code" << setw(22) << "Student Name"
       << setw(16) << "Exam Roll No" << setw(10) << "Program"
       << setw(10) << "Semester" << setw(12) << "Subject" << "\n";
  cout << string(84, '-') << "\n";

  int assigned = 0;
  int unassigned = 0;

  for (int r = 0; r < venue.rows; ++r) {
    for (int c = 0; c < venue.cols; ++c) {
      const Seat &seat = venue.layout[r][c];
      if (seat.studentIndex >= 0) {
        const Student &s = students[seat.studentIndex];
        cout << left << setw(14) << seat.seatCode << setw(22) << s.name
             << setw(16) << s.examRollNo << setw(10) << s.program
             << setw(10) << s.semester << setw(12) << s.subject << "\n";
        ++assigned;
      } else {
        ++unassigned;
      }
    }
  }

  cout << string(84, '-') << "\n";
  cout << "  Assigned: " << assigned << "  |  Vacant seats: " << unassigned
       << "  |  Total seats: " << venue.totalSeats() << "\n\n";

  // Print warnings
  if (!warnings.empty()) {
    cout << "  --- Warnings ---\n";
    for (const auto &w : warnings)
      cout << "  " << w << "\n";
    cout << "\n";
  }

  // Print visual grid with student programs
  cout << "  --- Visual Layout (by program) ---\n\n";
  // Column headers
  cout << "      ";
  for (int c = 0; c < venue.cols; ++c)
    cout << setw(8) << venue.columnLabel(c);
  cout << "\n";
  cout << "      " << string(venue.cols * 8, '-') << "\n";

  for (int r = 0; r < venue.rows; ++r) {
    cout << "R" << setw(2) << (r + 1) << " | ";
    for (int c = 0; c < venue.cols; ++c) {
      int idx = venue.layout[r][c].studentIndex;
      if (idx >= 0) {
        cout << setw(8) << students[idx].program;
      } else {
        cout << setw(8) << "---";
      }
    }
    cout << "\n";
  }
  cout << "\n";
}
