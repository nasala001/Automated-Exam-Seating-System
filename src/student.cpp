#include "../include/student.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void exportDatabaseToFinalCSV(sqlite3* DB) {
    // Final dynamic output file track open garne
    ofstream csvFile("data/Final_Seat_Plan.csv");
    if (!csvFile.is_open()) {
        cout << "[ERROR] Cannot open/create Final_Seat_Plan.csv! Check data/ folder." << endl;
        return;
    }
    
    // Naya CSV file ko top columns header line setup
    csvFile << "SN,Registration_ID,Roll_No,Name,Department,Semester,Program,Subject_Code,Assigned_Invigilators,Seat_Code\n";

    // Matrix pull garna main SELECT query text run gareko
    string query = "SELECT SN, REGID, ROLLNO, NAME, DEPARTMENT, SEMESTER, PROGRAM, SUBJECT_CODE, ASSIGNED_TEACHER, SEAT_CODE FROM SEAT_PLAN ORDER BY SN ASC;";
    sqlite3_stmt* stmt;

    // SELECT query prepare garera data target fetch garne loop logic
    if (sqlite3_prepare_v2(DB, query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        int currentSN = 1; 
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            // Null state values safe bypass validation checks
            string reg = (char*)sqlite3_column_text(stmt, 1) ? (char*)sqlite3_column_text(stmt, 1) : "";
            string roll = (char*)sqlite3_column_text(stmt, 2) ? (char*)sqlite3_column_text(stmt, 2) : "";
            string name = (char*)sqlite3_column_text(stmt, 3) ? (char*)sqlite3_column_text(stmt, 3) : "";
            string dept = (char*)sqlite3_column_text(stmt, 4) ? (char*)sqlite3_column_text(stmt, 4) : "";
            string sem = (char*)sqlite3_column_text(stmt, 5) ? (char*)sqlite3_column_text(stmt, 5) : "";
            string prog = (char*)sqlite3_column_text(stmt, 6) ? (char*)sqlite3_column_text(stmt, 6) : "";
            string sub = (char*)sqlite3_column_text(stmt, 7) ? (char*)sqlite3_column_text(stmt, 7) : "";
            string invigilator = (char*)sqlite3_column_text(stmt, 8) ? (char*)sqlite3_column_text(stmt, 8) : "";
            string seat = (char*)sqlite3_column_text(stmt, 9) ? (char*)sqlite3_column_text(stmt, 9) : "";

            // Dynamic format mapping anusar ofstream le naya file write garchha
            csvFile << currentSN << "," << reg << "," << roll << ",\"" << name << "\","
                    << dept << "," << sem << "," << prog << "," << sub << ",\""
                    << invigilator << "\"," << seat << "\n";
            
            currentSN++;
        }
        sqlite3_finalize(stmt);
        cout << "[SUCCESS] Final_Seat_Plan.csv generated successfully!" << endl;
    } else {
        cout << "[WARNING] SEAT_PLAN table template missing right now." << endl;
    }
    csvFile.close();
}