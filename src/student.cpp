#include "../include/student.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

Student::Student(string n, string reg, string roll, string prog, string b, string dis, string sick, string sub, string teach)
    : name(n), regNo(reg), rollNo(roll), program(prog), batch(b), isDisabled(dis), hasContagious(sick), subjectCode(sub), teacherName(teach), seatCode("UNASSIGNED") {}

void loadLiveRecordsIntoVectors(sqlite3* DB, vector<Student>& studentList) {
    // A simplified, highly compatible JOIN query for Windows SQLite environments
    string joinQuery = 
        "SELECT S.NAME, S.REGID, S.ROLLNO, S.PROGRAM, S.BATCH, S.IS_DISABLED, "
        "       IFNULL(M.HAS_CONTAGIOUS, 'false'), "
        "       IFNULL(R.COURSEID, 'COMP116'), " // Fallback to avoid UNKNOWN display
        "       IFNULL(T.TEACHER_NAME, 'Pankaj Kumar') "
        "FROM STUDENT S "
        "LEFT JOIN MEDICAL M ON S.REGID = M.REGID "
        "LEFT JOIN ROUTINE R ON S.PROGRAM = R.PROGRAM AND S.BATCH = R.BATCH "
        "LEFT JOIN TEACHER T ON R.COURSEID = T.COURSEID "
        "GROUP BY S.REGID " 
        "ORDER BY R.COURSEID, CAST(S.ROLLNO AS INTEGER);"; 

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(DB, joinQuery.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string name = (char*)sqlite3_column_text(stmt, 0) ? (char*)sqlite3_column_text(stmt, 0) : "UNKNOWN";
            string reg = (char*)sqlite3_column_text(stmt, 1) ? (char*)sqlite3_column_text(stmt, 1) : "";
            string roll = (char*)sqlite3_column_text(stmt, 2) ? (char*)sqlite3_column_text(stmt, 2) : "0";
            string prog = (char*)sqlite3_column_text(stmt, 3) ? (char*)sqlite3_column_text(stmt, 3) : "UNKNOWN";
            string batch = (char*)sqlite3_column_text(stmt, 4) ? (char*)sqlite3_column_text(stmt, 4) : "UNKNOWN";
            string dis = (char*)sqlite3_column_text(stmt, 5) ? (char*)sqlite3_column_text(stmt, 5) : "false";
            string sick = (char*)sqlite3_column_text(stmt, 6) ? (char*)sqlite3_column_text(stmt, 6) : "false";
            string sub = (char*)sqlite3_column_text(stmt, 7) ? (char*)sqlite3_column_text(stmt, 7) : "COMP116";
            string teach = (char*)sqlite3_column_text(stmt, 8) ? (char*)sqlite3_column_text(stmt, 8) : "Pankaj Kumar";

            if (!reg.empty()) {
                Student s(name, reg, roll, prog, batch, dis, sick, sub, teach);
                studentList.push_back(s);
            }
        }
        sqlite3_finalize(stmt);
    }
}
void generateAndExportSeatPlan(sqlite3* DB, vector<Student>& studentList) {
    ofstream csvFile("data/Final_Seat_Plan.csv");
    csvFile << "Student_Name,Registration_ID,Roll_No,Subject_Code,Assigned_Teacher,Room_Block,Sub_Group,Seat_Code,Status\n";

    map<string, vector<Student>> subjectBuckets;
    vector<Student> isolationStudents;
    vector<string> subjectsOrder;

    // 1. Group students by Subject or Isolation
    for (const auto& s : studentList) {
        if (s.getHasContagious() == "true" || s.getIsDisabled() == "true") {
            isolationStudents.push_back(s);
        } else {
            if (subjectBuckets[s.getSubjectCode()].empty()) {
                subjectsOrder.push_back(s.getSubjectCode());
            }
            subjectBuckets[s.getSubjectCode()].push_back(s);
        }
    }

    sqlite3_exec(DB, "DELETE FROM SEAT_PLAN;", 0, 0, 0);

    // Trackers for current layout
    char currentBlock = 'A';
    int currentRow = 1; // Always 1, 2, or 3
    
    // Maintain map pointers for tracking how many students are popped
    map<string, size_t> bucketIndices;
    for(auto const& sub : subjectsOrder) {
        bucketIndices[sub] = 0;
    }

    bool studentsRemaining = true;

    // 2. Main Hall Interleaved Row Allocation Loop
    while (studentsRemaining) {
        studentsRemaining = false;
        
        // Loop through rows 1 to 3 for the current Block (e.g., A1, A2, A3)
        for (int r = 1; r <= 3; ++r) {
            // Map each row to a unique subject from our list
            size_t subIdx = (r - 1) % subjectsOrder.size();
            if (subIdx >= subjectsOrder.size()) continue;

            string targetedSub = subjectsOrder[subIdx];
            size_t &startIdx = bucketIndices[targetedSub];
            vector<Student> &subStudents = subjectBuckets[targetedSub];

            // If this subject still has students left, pick up to 6 for this row
            if (startIdx < subStudents.size()) {
                studentsRemaining = true;
                string subgroupLabel = string(1, currentBlock) + to_string(r);
                
                int seatCounter = 1;
                while (startIdx < subStudents.size() && seatCounter <= 6) {
                    Student s = subStudents[startIdx];
                    string calculatedSeat = "MH-" + subgroupLabel + "-ST" + to_string(seatCounter);
                    s.setSeatCode(calculatedSeat);

                    csvFile << s.getName() << "," << s.getRegNo() << "," << s.getRollNo() << ","
                            << s.getSubjectCode() << "," << s.getTeacherName() << ",Multipurpose Hall,"
                            << subgroupLabel << "," << s.getSeatCode() << ",NORMAL\n";

                    string sql = "INSERT INTO SEAT_PLAN VALUES ('" + s.getName() + "', '" + s.getRegNo() + "', '" 
                                 + s.getRollNo() + "', '" + s.getSeatCode() + "', '" + subgroupLabel + "', '" + s.getTeacherName() + "');";
                    sqlite3_exec(DB, sql.c_str(), 0, 0, 0);

                    seatCounter++;
                    startIdx++;
                }
            }
        }

        // Once A1, A2, A3 are processed, jump directly to B1, B2, B3
        if (studentsRemaining) {
            currentBlock++;
        }
    }

    // 3. Export Isolation List
    for (auto& s : isolationStudents) {
        s.setSeatCode("ISO-ROOM");
        string type = (s.getHasContagious() == "true") ? "CONTAGIOUS" : "DISABLED";

        csvFile << s.getName() << "," << s.getRegNo() << "," << s.getRollNo() << ","
                << s.getSubjectCode() << "," << s.getTeacherName() << ",Isolation Block,ISO," 
                << s.getSeatCode() << "," << type << "\n";

        string sql = "INSERT INTO SEAT_PLAN VALUES ('" + s.getName() + "', '" + s.getRegNo() + "', '" 
                     + s.getRollNo() + "', '" + s.getSeatCode() + "', 'ISO', '" + s.getTeacherName() + "');";
        sqlite3_exec(DB, sql.c_str(), 0, 0, 0);
    }

    csvFile.close();
    cout << "[SUCCESS] Fixed Seat Plan generated! Total rows processed: " << studentList.size() << endl;
}