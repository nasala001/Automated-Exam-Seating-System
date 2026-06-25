#include "../include/student.hpp"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    sqlite3* DB;
    
    // Connect to embedded sqlite system file storage
    if (sqlite3_open("data/KU_Exam_System.db", &DB) != SQLITE_OK) {
        cout << "[ERROR] Cannot establish link connection to SQLite database storage!" << endl;
        return 1;
    }

    // Run custom migration wrapper engine
    if (runDatabaseMigrationEngine(DB)) {
        vector<Student> globalContainer;

        // Ingest updated dynamic records to system vector memory
        loadLiveRecordsIntoVectors(DB, globalContainer);

        // Export data pipe only if dynamic records exist
        if (!globalContainer.empty()) {
            generateAndExportSeatPlan(DB, globalContainer);
        } else {
            cout << "[WARNING] Vector ingestion contains 0 active student files!" << endl;
        }
    }

    sqlite3_close(DB);
    return 0;
}