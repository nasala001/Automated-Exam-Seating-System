#include "../include/student.hpp"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    sqlite3* DB;
    
    // Connect to the SQLite Database file
    if (sqlite3_open("data/KU_Exam_System.db", &DB) != SQLITE_OK) {
        cout << "[ERROR] Cannot connect to SQLite Database!" << endl;
        return 1;
    }

    // Run database tables initialization
    if (runDatabaseMigrationEngine(DB)) {
        vector<Student> globalContainer;

        // Fetch ALL 100-150 students from database into memory
        loadLiveRecordsIntoVectors(DB, globalContainer);

        // Generate final seating allocation if records exist
        if (!globalContainer.empty()) {
            generateAndExportSeatPlan(DB, globalContainer);
            cout << "[SUCCESS] Seating Plan generated successfully!" << endl;
        } else {
            cout << "[WARNING] No student records found in database!" << endl;
        }
    }

    sqlite3_close(DB);
    return 0;
}