#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "../database/sqlite3.h" // database फोल्डर भित्रको sqlite3.h तानेको

using namespace std;

int main() {
    sqlite3* DB;
    char* errMsg = nullptr;
    
    // data फोल्डर भित्र .db फाइल क्रिएट हुन्छ
    int exit = sqlite3_open("../data/KU_Exam_System.db", &DB);
    if (exit != SQLITE_OK) {
        cerr << "डाटाबेस खोल्न वा बनाउन सकिएन!" << endl;
        return -1;
    }
    cout << "[SUCCESS] Database opened/created successfully.\n";

    // टेबल स्ट्रक्चर
    string createTables = 
        "CREATE TABLE IF NOT EXISTS STUDENT ("
        "NAME TEXT, REGID TEXT PRIMARY KEY, ROLLNO TEXT, PROGRAM TEXT, BATCH TEXT, IS_DISABLED TEXT);"
        
        "CREATE TABLE IF NOT EXISTS MEDICAL ("
        "REGID TEXT PRIMARY KEY, HAS_CONTAGIOUS TEXT);"
        
        "CREATE TABLE IF NOT EXISTS TEACHER ("
        "COURSEID TEXT PRIMARY KEY, BATCH TEXT, TEACHER_NAME TEXT, CONTACT_NO TEXT);"
        
        "CREATE TABLE IF NOT EXISTS ROUTINE ("
        "COURSEID TEXT PRIMARY KEY, PROGRAM TEXT, BATCH TEXT, EXAM_DATE TEXT);";

    exit = sqlite3_exec(DB, createTables.c_str(), NULL, 0, &errMsg);
    if (exit != SQLITE_OK) {
        cerr << "टेबल बनाउन त्रुटि भयो: " << errMsg << endl;
        sqlite3_free(errMsg);
        return -1;
    }
    cout << "[SUCCESS] Tables created successfully.\n";

    sqlite3_exec(DB, "BEGIN TRANSACTION;", NULL, 0, NULL);

    // --- A. Student_Info.csv माइग्रेसन ---
    ifstream fileStud("../data/Student_Info.csv");
    if (fileStud.is_open()) {
        string line;
        getline(fileStud, line); 
        while (getline(fileStud, line)) {
            stringstream ss(line);
            string name, regId, rollNo, program, batch, isDisabled;

            getline(ss, name, ',');
            getline(ss, regId, ',');
            getline(ss, rollNo, ',');
            getline(ss, program, ',');
            getline(ss, batch, ',');
            getline(ss, isDisabled, ',');

            if (!regId.empty() && (program == "CE" || program == "Civil")) {
                string insertSQL = "INSERT OR IGNORE INTO STUDENT (NAME, REGID, ROLLNO, PROGRAM, BATCH, IS_DISABLED) VALUES ('" 
                                   + name + "', '" + regId + "', '" + rollNo + "', '" + program + "', '" + batch + "', '" + isDisabled + "');";
                sqlite3_exec(DB, insertSQL.c_str(), NULL, 0, NULL);
            }
        }
        fileStud.close();
        cout << "[SUCCESS] Student data migrated.\n";
    }

    // --- B. Medical_Log.csv माइग्रेसन ---
    ifstream fileMed("../data/Medical_Log.csv");
    if (fileMed.is_open()) {
        string line;
        getline(fileMed, line);
        while (getline(fileMed, line)) {
            stringstream ss(line);
            string regId, hasContagious;
            getline(ss, regId, ',');
            getline(ss, hasContagious, ',');

            if (!regId.empty()) {
                string insertSQL = "INSERT OR IGNORE INTO MEDICAL (REGID, HAS_CONTAGIOUS) VALUES ('" + regId + "', '" + hasContagious + "');";
                sqlite3_exec(DB, insertSQL.c_str(), NULL, 0, NULL);
            }
        }
        fileMed.close();
        cout << "[SUCCESS] Medical logs migrated.\n";
    }

    // --- C. Teacher_Info.csv माइग्रेसन ---
    ifstream fileTeach("../data/Teacher_Info.csv");
    if (fileTeach.is_open()) {
        string line;
        getline(fileTeach, line);
        while (getline(fileTeach, line)) {
            stringstream ss(line);
            string courseId, batch, teacherName, contactNo;
            getline(ss, courseId, ',');
            getline(ss, batch, ',');
            getline(ss, teacherName, ',');
            getline(ss, contactNo, ',');

            if (!courseId.empty()) {
                string insertSQL = "INSERT OR IGNORE INTO TEACHER (COURSEID, BATCH, TEACHER_NAME, CONTACT_NO) VALUES ('" 
                                   + courseId + "', '" + batch + "', '" + teacherName + "', '" + contactNo + "');";
                sqlite3_exec(DB, insertSQL.c_str(), NULL, 0, NULL);
            }
        }
        fileTeach.close();
        cout << "[SUCCESS] Teacher data migrated.\n";
    }

    // --- D. Exam_Routine.csv माइग्रेसन ---
    ifstream fileRoutine("../data/Exam_Routine.csv");
    if (fileRoutine.is_open()) {
        string line;
        getline(fileRoutine, line);
        while (getline(fileRoutine, line)) {
            stringstream ss(line);
            string courseId, program, batch, examDate;
            getline(ss, courseId, ',');
            getline(ss, program, ',');
            getline(ss, batch, ',');
            getline(ss, examDate, ',');

            if (!courseId.empty() && (program == "CE" || program == "Civil")) {
                string insertSQL = "INSERT OR IGNORE INTO ROUTINE (COURSEID, PROGRAM, BATCH, EXAM_DATE) VALUES ('" 
                                   + courseId + "', '" + program + "', '" + batch + "', '" + examDate + "');";
                sqlite3_exec(DB, insertSQL.c_str(), NULL, 0, NULL);
            }
        }
        fileRoutine.close();
        cout << "[SUCCESS] Exam routine migrated.\n";
    }

    sqlite3_exec(DB, "COMMIT;", NULL, 0, NULL);
    cout << "\n[MIGRATION COMPLETE] All data perfectly synced.\n";

    sqlite3_close(DB);
    return 0;
}