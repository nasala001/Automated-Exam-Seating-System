#include "SeatCodeUtils.h"
#include "Venue.h"

#include <iostream>
#include <limits>
#include <string>

using namespace std;

// ---------------------------------------------------------------------------
// Input helpers
// ---------------------------------------------------------------------------

static int readPositiveInt(const string &prompt) {
  int value;
  while (true) {
    cout << prompt;
    if (cin >> value && value > 0) {
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      return value;
    }
    cout << "  [!] Please enter a positive integer.\n";
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
  }
}

static string readNonEmpty(const string &prompt) {
  string value;
  while (true) {
    cout << prompt;
    getline(cin, value);
    if (!value.empty())
      return value;
    cout << "  [!] Input cannot be empty.\n";
  }
}

// ---------------------------------------------------------------------------
// Main menu
// ---------------------------------------------------------------------------

int main() {
  cout << "==============================================\n";
  cout << "   Automated Exam Seating - Seat Code Demo\n";
  cout << "==============================================\n\n";

  while (true) {
    cout << "Select venue type:\n";
    cout << "  [1] Exam Room  (e.g. Block 8, Room 304)\n";
    cout << "  [2] Large Hall (e.g. Multipurpose Hall, Group D)\n";
    cout << "  [0] Exit\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 0) {
      cout << "\nGoodbye!\n";
      break;
    }

    if (choice == 1) {
      // ----- Exam Room -----
      string blockName =
          readNonEmpty("\nEnter block/building name (e.g. \"Block 9\"): ");
      string roomNo = readNonEmpty("Enter room number (e.g. \"304\"): ");
      int rows = readPositiveInt("Enter number of rows    : ");
      int cols = readPositiveInt("Enter number of columns : ");

      ExamRoom room(blockName, roomNo, rows, cols);
      room.generateLayout();
      room.printLayout();

    } else if (choice == 2) {
      // ----- Large Hall -----
      string hallName =
          readNonEmpty("\nEnter hall name (e.g. \"Multipurpose Hall\"): ");
      string group = readNonEmpty("Enter group/section label (e.g. \"D\"): ");
      int rows = readPositiveInt("Enter number of rows    : ");
      int cols = readPositiveInt("Enter number of columns : ");

      LargeHall hall(hallName, group, rows, cols);
      hall.generateLayout();
      hall.printLayout();

    } else {
      cout << "\n  [!] Invalid choice. Please enter 0, 1, or 2.\n\n";
    }
  }

  return 0;
}
