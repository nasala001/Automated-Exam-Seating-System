#include "../include/student.hpp"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    sqlite3* DB;
    
    // Connect to local architecture database logs
    if (sqlite3_open("data/KU_Exam_System.db", &DB) != SQLITE_OK) {
        cout << "[ERROR] Cannot establish link connection to SQLite database storage!" << endl;
        return 1;
    }

    // Run execution chain
    if (runDatabaseMigrationEngine(DB)) {
        vector<Student> globalStudentVector;

        // Populate system records into dynamic vectors via SQL LEFT JOIN
        loadLiveRecordsIntoVectors(DB, globalStudentVector);

        // Apply rules layout and dump final CSV profiles
        if (!globalStudentVector.empty()) {
            generateAndExportSeatPlan(DB, globalStudentVector);
        } else {
            cout << "[WARNING] Vector ingestion contains 0 active student files!" << endl;
        }
    }

    sqlite3_close(DB);
    return 0;
}