#include "../include/student.hpp"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    sqlite3* DB;
    
    if (sqlite3_open("data/KU_Exam_System.db", &DB) != SQLITE_OK) {
        cout << "[ERROR] Cannot establish link connection to SQLite database storage!" << endl;
        return 1;
    }

    if (runDatabaseMigrationEngine(DB)) {
        vector<Student> globalContainer;

        loadLiveRecordsIntoVectors(DB, globalContainer);

        if (!globalContainer.empty()) {
            generateAndExportSeatPlan(DB, globalContainer);
        } else {
            cout << "[WARNING] Vector ingestion contains 0 active student files!" << endl;
        }
    }

    sqlite3_close(DB);
    return 0;
}