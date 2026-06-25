#include "../include/student.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

// Initialize student object with details
Student::Student(int id, string n, string reg, string roll, string dept, string sem, string prog, string b, string dis, string sick, string sub, string teach)
    : sn(id), name(n), regNo(reg), rollNo(roll), department(dept), semester(sem), program(prog), batch(b), isDisabled(dis), hasContagious(sick), subjectCode(sub), teacherName(teach), seatCode("UNASSIGNED") {}

// Match room subgroup with specific invigilator
string getAssignedTeacher(const string& seatCode) {
    if (seatCode == "ISO-ROOM") {
        return "Medical Team Incharge";
    }

    // Get block and row prefix like "MH-A1"
    string subGroup = seatCode.substr(0, 5);

    // Dynamic map to look up teachers for each sub-block
    map<string, string> teacherMap = {
        {"MH-A1", "Prof. Ram Swarth"},
        {"MH-A2", "Dr. Sunita Sharma"},
        {"MH-A3", "Er. Rajesh Gupta"},
        {"MH-B1", "Prof. KP Mishra"},
        {"MH-B2", "Dr. Niranjan Joshi"},
        {"MH-B3", "Er. Sandesh Shrestha"},
        {"MH-C1", "Mrs. Anjana Regmi"},
        {"MH-C2", "Dr. Santosh Bhandari"},
        {"MH-C3", "Er. Dipendra Shah"}
    };

    if (teacherMap.find(subGroup) != teacherMap.end()) {
        return teacherMap[subGroup];
    }
    
    return "Pankaj Kumar"; // Fallback default teacher
}

// Fetch live database records using SQL join operation
void loadLiveRecordsIntoVectors(sqlite3* DB, vector<Student>& studentList) {
    // Left join query to pull relational data together cleanly
    string joinQuery = 
        "SELECT S.SN, S.NAME, S.REGID, S.ROLLNO, S.DEPARTMENT, S.SEMESTER, S.PROGRAM, S.BATCH, S.IS_DISABLED, "
        "       IFNULL(M.HAS_CONTAGIOUS, 'false'), "
        "       IFNULL(R.COURSEID, 'COMP116'), " 
        "       IFNULL(T.TEACHER_NAME, 'Pankaj Kumar') "
        "FROM STUDENT S "
        "LEFT JOIN MEDICAL M ON S.REGID = M.REGID "
        "LEFT JOIN ROUTINE R ON S.DEPARTMENT = R.PROGRAM AND S.BATCH = R.BATCH "
        "LEFT JOIN TEACHER T ON R.COURSEID = T.COURSEID "
        "ORDER BY S.SN ASC;"; 

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(DB, joinQuery.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        // Read database row by row
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            string name = (char*)sqlite3_column_text(stmt, 1) ? (char*)sqlite3_column_text(stmt, 1) : "UNKNOWN";
            string reg = (char*)sqlite3_column_text(stmt, 2) ? (char*)sqlite3_column_text(stmt, 2) : "";
            string roll = (char*)sqlite3_column_text(stmt, 3) ? (char*)sqlite3_column_text(stmt, 3) : "0";
            string dept = (char*)sqlite3_column_text(stmt, 4) ? (char*)sqlite3_column_text(stmt, 4) : "Computer Science";
            string sem = (char*)sqlite3_column_text(stmt, 5) ? (char*)sqlite3_column_text(stmt, 5) : "Sem 1";
            string prog = (char*)sqlite3_column_text(stmt, 6) ? (char*)sqlite3_column_text(stmt, 6) : "BE";
            string batch = (char*)sqlite3_column_text(stmt, 7) ? (char*)sqlite3_column_text(stmt, 7) : "2025";
            string dis = (char*)sqlite3_column_text(stmt, 8) ? (char*)sqlite3_column_text(stmt, 8) : "false";
            string sick = (char*)sqlite3_column_text(stmt, 9) ? (char*)sqlite3_column_text(stmt, 9) : "false";
            string sub = (char*)sqlite3_column_text(stmt, 10) ? (char*)sqlite3_column_text(stmt, 10) : "COMP116";
            string teach = (char*)sqlite3_column_text(stmt, 11) ? (char*)sqlite3_column_text(stmt, 11) : "Pankaj Kumar";

            if (!reg.empty()) {
                Student s(id, name, reg, roll, dept, sem, prog, batch, dis, sick, sub, teach);
                studentList.push_back(s);
            }
        }
        sqlite3_finalize(stmt);
    }
}

// Generate the output CSV format and dump to file system
void generateAndExportSeatPlan(sqlite3* DB, vector<Student>& studentList) {
    ofstream csvFile("data/Final_Seat_Plan.csv");
    
    // CSV Header mapping UI layout
    csvFile << "SN,Registration_ID,Roll_No,Name,Department,Semester,Program,Subject_Code,Assigned_Teacher,Seat_Code\n";

    map<string, vector<Student>> subjectBuckets;
    vector<Student> isolationStudents;
    vector<string> subjectsOrder;

    // Separate normal students from isolation candidates
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

    // Refresh old seating data from tables
    sqlite3_exec(DB, "DELETE FROM SEAT_PLAN;", 0, 0, 0);

    char currentBlock = 'A';
    int maxSeatsPerRow = 12; 
    
    map<string, size_t> bucketIndices;
    for(auto const& sub : subjectsOrder) {
        bucketIndices[sub] = 0;
    }

    bool studentsRemaining = true;

    // Main hall seating layout calculation loops
    while (studentsRemaining && currentBlock <= 'I') {
        studentsRemaining = false;
        
        for (int r = 1; r <= 3; ++r) {
            if(subjectsOrder.empty()) break;
            size_t subIdx = (r - 1) % subjectsOrder.size();

            string targetedSub = subjectsOrder[subIdx];
            size_t &startIdx = bucketIndices[targetedSub];
            vector<Student> &subStudents = subjectBuckets[targetedSub];

            if (startIdx < subStudents.size()) {
                studentsRemaining = true;
                string subgroupLabel = string(1, currentBlock) + to_string(r);
                
                int seatCounter = 1;
                while (startIdx < subStudents.size() && seatCounter <= maxSeatsPerRow) {
                    Student s = subStudents[startIdx];
                    string calculatedSeat = "MH-" + subgroupLabel + "-ST" + to_string(seatCounter);
                    s.setSeatCode(calculatedSeat);

                    // Map specific teacher dynamically depending on block location
                    string dynamicTeacher = getAssignedTeacher(calculated