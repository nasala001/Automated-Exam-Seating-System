#include "Venue.h"
#include "SeatCodeUtils.h"

#include <iomanip>
#include <iostream>
#include <numeric>
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
      layout[r][c].studentIndex = -1;
    }
  }
}

string Venue::columnLabel(int colIndex) const {
  // Base implementation: "Col 1", "Col 2", ...
  return "Col " + to_string(colIndex + 1);
}

void Venue::printLayout() const {
  cout << "\n  Venue  : " << name << "  [" << code << "]\n";
  cout << "  Layout : " << rows << " rows x " << cols << " columns  ("
       << rows * cols << " seats total)\n\n";

  // Determine the widest seat code for column alignment
  size_t maxCodeLen = 0;
  for (const auto &row : layout)
    for (const auto &seat : row)
      if (seat.seatCode.size() > maxCodeLen)
        maxCodeLen = seat.seatCode.size();

  // Also check column label widths
  size_t maxLabelLen = 0;
  for (int c = 0; c < cols; ++c) {
    size_t len = columnLabel(c).size();
    if (len > maxLabelLen)
      maxLabelLen = len;
  }

  const int cellWidth =
      static_cast<int>(max(maxCodeLen, maxLabelLen)) + 2;

  // Column header
  cout << string(5, ' ');
  for (int c = 0; c < cols; ++c)
    cout << setw(cellWidth) << columnLabel(c);
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

LargeHall::LargeHall(const string &hallName,
                     const vector<Section> &secs,
                     int r)
    : Venue(hallName, r,
            // total cols = sum of all section column counts
            accumulate(secs.begin(), secs.end(), 0,
                       [](int sum, const Section &s) { return sum + s.cols; })),
      sections(secs) {}

void LargeHall::generateLayout() {
  initGrid();
  for (auto &row : layout) {
    for (auto &seat : row) {
      pair<string, int> resolved = resolveColumn(seat.col - 1);
      // Format: HALLCODE-SECTION{colInSection}-{row}
      // e.g. "MPH-A1-5"
      seat.seatCode =
          code + "-" + resolved.first + to_string(resolved.second) + "-" + to_string(seat.row);
    }
  }
}

string LargeHall::columnLabel(int colIndex) const {
  pair<string, int> resolved = resolveColumn(colIndex);
  return "Col " + resolved.first + to_string(resolved.second);
}

pair<string, int> LargeHall::resolveColumn(int colIndex) const {
  int offset = 0;
  for (const auto &sec : sections) {
    if (colIndex < offset + sec.cols) {
      int colInSection = (colIndex - offset) + 1; // 1-indexed
      return {sec.label, colInSection};
    }
    offset += sec.cols;
  }
  // Fallback (should never reach here if colIndex is valid)
  return {"?", colIndex + 1};
}
