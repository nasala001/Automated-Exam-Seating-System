#include "SeatCodeUtils.h"
#include "SeatingEngine.h"
#include "Student.h"
#include "Venue.h"

#include <iostream>
#include <limits>
#include <string>
#include <vector>

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
  cout << "   Automated Exam Seating System\n";
  cout << "==============================================\n\n";

  while (true) {
    cout << "Select an option:\n";
    cout << "  [1] Generate Exam Room Layout\n";
    cout << "  [2] Generate Large Hall Layout\n";
    cout << "  [3] Run Seating Assignment\n";
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
      // ----- Large Hall (with sections) -----
      string hallName =
          readNonEmpty("\nEnter hall name (e.g. \"Multipurpose Hall\"): ");
      int numSections = readPositiveInt("Enter number of sections: ");
      int seatsPerSection = readPositiveInt("Enter number of students per section: ");
      int colsPerSection = readPositiveInt("Enter number of columns per section: ");

      // Auto-calculate rows: ceil(seatsPerSection / colsPerSection)
      int rows = (seatsPerSection + colsPerSection - 1) / colsPerSection;

      vector<Section> sections;
      for (int i = 0; i < numSections; ++i) {
        string label(1, 'A' + i); // A, B, C, ...
        sections.push_back({label, colsPerSection});
      }

      cout << "\n  Sections created: ";
      for (int i = 0; i < numSections; ++i) {
        if (i > 0) cout << ", ";
        cout << sections[i].label << " (" << colsPerSection << " cols)";
      }
      cout << "\n  Rows calculated : " << rows
           << "  (" << seatsPerSection << " students / "
           << colsPerSection << " cols)\n";

      LargeHall hall(hallName, sections, rows);
      hall.generateLayout();
      hall.printLayout();

    } else if (choice == 3) {
      // ----- Seating Assignment -----
      cout << "\n--- Seating Assignment ---\n\n";

      // Step 1: Load students
      string csvPath =
          readNonEmpty("Enter path to student CSV file: ");
      vector<Student> students = parseStudentsFromCSV(csvPath);
      if (students.empty()) {
        cout << "  [!] No students loaded. Returning to menu.\n\n";
        continue;
      }

      // Step 2: Choose venue type
      cout << "\nSelect venue type:\n";
      cout << "  [1] Exam Room\n";
      cout << "  [2] Large Hall\n";
      cout << "Choice: ";
      int venueChoice;
      cin >> venueChoice;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');

      Venue *venue = nullptr;

      if (venueChoice == 1) {
        string blockName =
            readNonEmpty("\nEnter block/building name: ");
        string roomNo = readNonEmpty("Enter room number: ");
        int rows = readPositiveInt("Enter number of rows    : ");
        int cols = readPositiveInt("Enter number of columns : ");

        venue = new ExamRoom(blockName, roomNo, rows, cols);

      } else if (venueChoice == 2) {
        string hallName =
            readNonEmpty("\nEnter hall name: ");
        int numSections = readPositiveInt("Enter number of sections: ");
        int seatsPerSection = readPositiveInt("Enter number of students per section: ");
        int colsPerSection = readPositiveInt("Enter number of columns per section: ");

        // Auto-calculate rows
        int rows = (seatsPerSection + colsPerSection - 1) / colsPerSection;

        vector<Section> sections;
        for (int i = 0; i < numSections; ++i) {
          string label(1, 'A' + i); // A, B, C, ...
          sections.push_back({label, colsPerSection});
        }

        cout << "\n  Sections created: ";
        for (int i = 0; i < numSections; ++i) {
          if (i > 0) cout << ", ";
          cout << sections[i].label << " (" << colsPerSection << " cols)";
        }
        cout << "\n  Rows calculated : " << rows
             << "  (" << seatsPerSection << " students / "
             << colsPerSection << " cols)\n";

        venue = new LargeHall(hallName, sections, rows);

      } else {
        cout << "  [!] Invalid venue choice.\n\n";
        continue;
      }

      // Step 3: Generate layout and assign seats
      venue->generateLayout();

      cout << "\n  Venue: " << venue->name << " [" << venue->code << "]"
           << "  (" << venue->rows << " rows x " << venue->cols << " cols = "
           << venue->totalSeats() << " seats)\n";
      cout << "  Students: " << students.size() << "\n\n";

      SeatingEngine engine;
      bool ok = engine.assignSeats(*venue, students);

      if (ok) {
        // Print the seat code layout
        venue->printLayout();

        // Print the full seating plan
        engine.printSeatingPlan(*venue, students);
      } else {
        cout << "  [!] Assignment failed.\n";
        for (const auto &w : engine.getWarnings())
          cout << "  " << w << "\n";
      }

      delete venue;
      cout << "\n";

    } else {
      cout << "\n  [!] Invalid choice. Please enter 0, 1, 2, or 3.\n\n";
    }
  }

  return 0;
}
