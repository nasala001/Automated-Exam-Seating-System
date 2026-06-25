#include "../include/student.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

bool runDatabaseMigrationEngine(sqlite3* DB) {
    char* err = nullptr;
    
    // Schema with 10-column structure matching UI properties
    string schema = 
        "DROP TABLE IF EXISTS STUDENT; DROP TABLE IF EXISTS MEDICAL;"
        "DROP TABLE IF EXISTS TEACHER; DROP TABLE IF EXISTS ROUTINE; DROP TABLE IF EXISTS SEAT_PLAN;"
        
        "CREATE TABLE STUDENT (SN INTEGER PRIMARY KEY, REGID TEXT, ROLLNO TEXT, NAME TEXT, DEPARTMENT TEXT, SEMESTER TEXT, PROGRAM TEXT, BATCH TEXT, IS_DISABLED TEXT);"
        "CREATE TABLE MEDICAL (REGID TEXT PRIMARY KEY, HAS_CONTAGIOUS TEXT);"
        "CREATE TABLE TEACHER (COURSEID TEXT PRIMARY KEY, BATCH TEXT, TEACHER_NAME TEXT, CONTACT_NO TEXT);"
        "CREATE TABLE ROUTINE (COURSEID TEXT PRIMARY KEY, PROGRAM TEXT, BATCH TEXT, EXAM_DATE TEXT);"
        "CREATE TABLE SEAT_PLAN (SN INTEGER, REGID TEXT, ROLLNO TEXT, NAME TEXT, DEPARTMENT TEXT, SEMESTER TEXT, PROGRAM TEXT, SUBJECT_CODE TEXT, ASSIGNED_TEACHER TEXT, SEAT_CODE TEXT);";

    if (sqlite3_exec(DB, schema.c_str(), 0, 0, &err) != SQLITE_OK) {
        cout << "[ERROR] Schema initialization failed: " << err << endl;
        sqlite3_free(err);
        return false;
    }

    int studentCount = 0, medicalCount = 0, routineCount = 0, teacherCount = 0;
    string line;

    // 1. Migrate using exact name from image_3dd70c.jpg
    ifstream f1("data/Student_Info.csv");
    if (!f1.is_open()) {
        cout << "[CRITICAL ERROR] data/Student_Info.csv not found!" << endl;
    } else {
        getline(f1, line); // Skip headers
        while (getline(f1, line)) {
            stringstream ss(line); string sn, reg, r, n, dept, sem, p, b, d;
            getline(ss, sn, ','); getline(ss, reg, ','); getline(ss, r, ',');
            getline(ss, n, ','); getline(ss, dept, ','); getline(ss, sem, ',');
            getline(ss, p, ','); getline(ss, b, ','); getline(ss, d, ',');
            
            if(!reg.empty()) {
                string sql = "INSERT OR IGNORE INTO STUDENT VALUES ("+sn+",'"+reg+"','"+r+"','"+n+"','"+dept+"','"+sem+"','"+p+"','"+b+"','"+d+"');";
                if(sqlite3_exec(DB, sql.c_str(), 0, 0, 0) == SQLITE_OK) studentCount++;
            }
        }
        f1.close();
    }

    // 2. Migrate using exact name from image_3dd70c.jpg
    ifstream f2("data/Medical_Log.csv");
    if (!f2.is_open()) {
        cout << "[WARNING] data/Medical_Log.csv not found!" << endl;
    } else {
        getline(f2, line);
        while (getline(f2, line)) {
            stringstream ss(line); string reg, sick;
            getline(ss, reg, ','); getline(ss, sick, ',');
            if(!reg.empty()) {
                string sql = "INSERT OR IGNORE INTO MEDICAL VALUES ('"+reg+"','"+sick+"');";
                if(sqlite3_exec(DB, sql.c_str(), 0, 0, 0) == SQLITE_OK) medicalCount++;
            }
        }
        f2.close();
    }

    // 3. Migrate using exact name from image_3dd70c.jpg
    ifstream f3("data/Exam_Routine.csv");
    if (!f3.is_open()) {
        cout << "[WARNING] data/Exam_Routine.csv not found!" << endl;
    } else {
        getline(f3, line);
        while (getline(f3, line)) {
            stringstream ss(line); string cid, prog, batch;
            getline(ss, cid, ','); getline(ss, prog, ','); getline(ss, batch, ',');
            if(!cid.empty()) {
                string sql = "INSERT OR IGNORE INTO ROUTINE VALUES ('"+cid+"','"+prog+"','"+batch+"','2026-06-25');";
                if(sqlite3_exec(DB, sql.c_str(), 0, 0, 0) == SQLITE_OK) routineCount++;
            }
        }
        f3.close();
    }

    // 4. Migrate using exact name from image_3dd70c.jpg
    ifstream f4("data/Teacher_Info.csv");
    if (!f4.is_open()) {
        cout << "[WARNING] data/Teacher_Info.csv not found!" << endl;
    } else {
        getline(f4, line);
        while (getline(f4, line)) {
            stringstream ss(line); string cid, tname;
            getline(ss, cid, ','); getline(ss, tname, ',');
            if(!cid.empty()) {
                string sql = "INSERT OR IGNORE INTO TEACHER VALUES ('"+cid+"','2025','"+tname+"','9841000000');";
                if(sqlite3_exec(DB, sql.c_str(), 0, 0, 0) == SQLITE_OK) teacherCount++;
            }
        }
        f4.close();
    }

    cout << "\n--- DB MIGRATION REPORT ---" << endl;
    cout << "STUDENT Table Rows Inserted: " << studentCount << endl;
    cout << "MEDICAL Table Rows Inserted: " << medicalCount << endl;
    cout << "ROUTINE Table Rows Inserted: " << routineCount << endl;
    cout << "TEACHER Table Rows Inserted: " << teacherCount << endl;
    cout << "---------------------------\n" << endl;

    return true;
}