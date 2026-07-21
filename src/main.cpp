#include "../include/student.hpp"
#include <iostream>

using namespace std;

int main() {
    sqlite3* DB;
    
    // Main KU local SQLite Database connection file path trigger
    if (sqlite3_open("data/KU_Exam_System.db", &DB) != SQLITE_OK) {
        cout << "[ERROR] Cannot open SQLite Database!" << endl;
        return 1;
    }

    cout << "=== KU AUTOMATED EXAM SEATING SYSTEM ===" << endl;

    // Step 1: Raw CSV files check garera SQLite table ma load handine
    if (runDatabaseMigrationEngine(DB)) {
        
        cout << "[SYSTEM] Waiting for main logical algorithm to fill SEAT_PLAN table..." << endl;
        // --------------------------------------------------------------------------
        // Aashish & Shresh's logic engine runs rules and fills database here...
        // --------------------------------------------------------------------------

        // Step 2: Rules validation run bhayeshi direct final CSV export dynamic push garne
        exportDatabaseToFinalCSV(DB);
    }

    sqlite3_close(DB);
    return 0;
}