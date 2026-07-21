#include "../include/student.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// Name ma complex characters (like single quote) aayo bhane error fix garne function
string escapeSQL(const string& target) {
    string result = "";
    for (char c : target) {
        if (c == '\'') result += "''";
        else result += c;
    }
    return result;
}

bool runDatabaseMigrationEngine(sqlite3* DB) {
    char* err = nullptr;
    
    // Purano table delete garne ra dynamic naya tables fresh banaune SQL script
    string schema = 
        "DROP TABLE IF EXISTS STUDENT; DROP TABLE IF EXISTS MEDICAL;"
        "DROP TABLE IF EXISTS TEACHER; DROP TABLE IF EXISTS ROUTINE;"
        "CREATE TABLE STUDENT (SN INTEGER, REGID TEXT PRIMARY KEY, ROLLNO TEXT, NAME TEXT, DEPARTMENT TEXT, SEMESTER TEXT, PROGRAM TEXT, BATCH TEXT, IS_DISABLED TEXT);"
        "CREATE TABLE MEDICAL (REGID TEXT PRIMARY KEY, HAS_CONTAGIOUS TEXT);"
        "CREATE TABLE TEACHER (COURSEID TEXT PRIMARY KEY, TEACHER_NAME TEXT);"
        "CREATE TABLE ROUTINE (COURSEID TEXT PRIMARY KEY, PROGRAM TEXT, BATCH TEXT, EXAM_DATE TEXT);";

    // SQL tables check garera run garne
    if (sqlite3_exec(DB, schema.c_str(), 0, 0, &err) != SQLITE_OK) {
        cout << "[ERROR] Database setup failed: " << err << endl;
        sqlite3_free(err);
        return false;
    }

    int studentCount = 0, medicalCount = 0, routineCount = 0, teacherCount = 0;
    string line;

    // A. Student_Info.csv load garne space
    ifstream f1("data/Student_Info.csv");
    if (f1.is_open()) {
        getline(f1, line); // Heading row skip gareko
        while (getline(f1, line)) {
            stringstream ss(line); string sn, reg, r, n, dept, sem, p, b, d;
            getline(ss, sn, ','); getline(ss, reg, ','); getline(ss, r, ',');
            getline(ss, n, ','); getline(ss, dept, ','); getline(ss, sem, ',');
            getline(ss, p, ','); getline(ss, b, ','); getline(ss, d, ',');
            
            if(!reg.empty()) {
                string sql = "INSERT OR IGNORE INTO STUDENT VALUES (" + sn + ",'" + reg + "','" + r + "','" + escapeSQL(n) + "','" + dept + "','" + sem + "','" + p + "','" + b + "','" + d + "');";
                if(sqlite3_exec(DB, sql.c_str(), 0, 0, 0) == SQLITE_OK) studentCount++;
            }
        }
        f1.close();
    }

    // B. Medical_Log.csv load garne space
    ifstream f2("data/Medical_Log.csv");
    if (f2.is_open()) {
        getline(f2, line); // Heading row skip gareko
        while (getline(f2, line)) {
            stringstream ss(line); string reg, sick;
            getline(ss, reg, ','); getline(ss, sick, ',');
            if(!reg.empty()) {
                string sql = "INSERT OR IGNORE INTO MEDICAL VALUES ('" + reg + "','" + sick + "');";
                if(sqlite3_exec(DB, sql.c_str(), 0, 0, 0) == SQLITE_OK) medicalCount++;
            }
        }
        f2.close();
    }

    // C. Exam_Routine.csv load garne space
    ifstream f3("data/Exam_Routine.csv");
    if (f3.is_open()) {
        getline(f3, line); // Heading row skip gareko
        while (getline(f3, line)) {
            stringstream ss(line); string cid, prog, batch, examDate;
            getline(ss, cid, ','); getline(ss, prog, ','); getline(ss, batch, ','); getline(ss, examDate, ',');
            if(!cid.empty()) {
                string sql = "INSERT OR IGNORE INTO ROUTINE VALUES ('" + cid + "','" + prog + "','" + batch + "','" + examDate + "');";
                if(sqlite3_exec(DB, sql.c_str(), 0, 0, 0) == SQLITE_OK) routineCount++;
            }
        }
        f3.close();
    }

    // D. Teacher_Info.csv load garne space
    ifstream f4("data/Teacher_Info.csv");
    if (f4.is_open()) {
        getline(f4, line); // Heading row skip gareko
        while (getline(f4, line)) {
            stringstream ss(line); string courseId, tname;
            getline(ss, courseId, ','); getline(ss, tname, ',');
            if(!courseId.empty()) {
                string sql = "INSERT OR IGNORE INTO TEACHER VALUES ('" + courseId + "','" + escapeSQL(tname) + "');";
                if(sqlite3_exec(DB, sql.c_str(), 0, 0, 0) == SQLITE_OK) teacherCount++;
            }
        }
        f4.close();
    }

    // Console screen ma final report print garne
    cout << "\n--- DATA MIGRATION REPORT ---" << endl;
    cout << "STUDENT Rows Added: " << studentCount << " | MEDICAL Rows Added: " << medicalCount << endl;
    cout << "ROUTINE Rows Added: " << routineCount << " | TEACHER Rows Added: " << teacherCount << endl;
    cout << "-----------------------------\n" << endl;

    return true;
}