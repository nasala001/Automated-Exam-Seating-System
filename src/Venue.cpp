#include "Venue.h"
#include "SeatCodeUtils.h"

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

// ---------------------------------------------------------------------------
// Venue  (base)
// ---------------------------------------------------------------------------

Venue::Venue(const string &fullName, int r, int c)
    : name(fullName), code(normalizeVenueName(fullName)), rows(r), cols(c) {}

void Venue::initGrid() {
  layout.clear();
  layout.resize(rows, vector<Seat>(cols));
  int seatNum = 1;
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      layout[r][c].row = r + 1;
      layout[r][c].col = c + 1;
      layout[r][c].seatNumber = seatNum++;
      layout[r][c].seatCode = ""; // filled in by derived class
    }
  }
}

void Venue::printLayout() const {
  cout << "\n  Venue  : " << name << "  [" << code << "]\n";
  cout << "  Layout : " << rows << " rows x " << cols << " columns  ("
       << rows * cols << " seats total)\n\n";

  // Determine the widest seat code for column alignment
  size_t maxLen = 0;
  for (const auto &row : layout)
    for (const auto &seat : row)
      if (seat.seatCode.size() > maxLen)
        maxLen = seat.seatCode.size();

  const int cellWidth = static_cast<int>(maxLen) + 2;

  // Column header
  cout << string(5, ' ');
  for (int c = 1; c <= cols; ++c)
    cout << setw(cellWidth) << ("Col " + to_string(c));
  cout << "\n";

  cout << string(5, ' ') << string(cols * cellWidth, '-') << "\n";

  // Rows
  for (int r = 0; r < rows; ++r) {
    cout << "R" << setw(2) << (r + 1) << " |";
    for (int c = 0; c < cols; ++c)
      cout << setw(cellWidth) << layout[r][c].seatCode;
    cout << "\n";
  }
  cout << "\n";
}

// ---------------------------------------------------------------------------
// ExamRoom
// ---------------------------------------------------------------------------

ExamRoom::ExamRoom(const string &blockName, const string &roomNo, int r, int c)
    : Venue(blockName, r, c), roomNumber(roomNo) {}

void ExamRoom::generateLayout() {
  initGrid();
  for (auto &row : layout) {
    for (auto &seat : row) {
      // Format: BLOCKCODE-ROOMNO-SEATNO  (e.g. "B8-304-12")
      seat.seatCode =
          code + "-" + roomNumber + "-" + to_string(seat.seatNumber);
    }
  }
}

// ---------------------------------------------------------------------------
// LargeHall
// ---------------------------------------------------------------------------

LargeHall::LargeHall(const string &hallName, const string &grp, int r, int c)
    : Venue(hallName, r, c), group(grp) {}

void LargeHall::generateLayout() {
  initGrid();
  for (auto &row : layout) {
    for (auto &seat : row) {
      // Format: HALLCODE-GROUP-SEATNO  (e.g. "MPH-D-21")
      seat.seatCode = code + "-" + group + "-" + to_string(seat.seatNumber);
    }
  }
}
