#include "../include/student.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <cmath>

using namespace std;

// Initialize Student Object
Student::Student(int id, string n, string reg, string roll, string dept, string sem, string prog, string b, string dis, string sick, string sub, string teach)
    : sn(id), name(n), regNo(reg), rollNo(roll), department(dept), semester(sem), program(prog), batch(b), isDisabled(dis), hasContagious(sick), subjectCode(sub), teacherName(teach), seatCode("UNASSIGNED") {}

// Fetch Assigned Teacher for a specific course from database
string getAssignedTeacherFromDB(sqlite3* DB, const string& subjectCode) {
    string teacherName = "Prof. Pankaj Kumar"; // Default fallback teacher
    string query = "SELECT TEACHER_NAME FROM TEACHER WHERE COURSEID = '" + subjectCode + "';";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(DB, query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            teacherName = (char*)sqlite3_column_text(stmt, 0) ? (char*)sqlite3_column_text(stmt, 0) : "Prof. Pankaj Kumar";
        }
        sqlite3_finalize(stmt);
    }
    return teacherName;
}

// Fetch all students using LEFT JOIN so no student is dropped from the list
void loadLiveRecordsIntoVectors(sqlite3* DB, vector<Student>& studentList) {
    string joinQuery = 
        "SELECT S.SN, S.NAME, S.REGID, S.ROLLNO, S.DEPARTMENT, S.SEMESTER, S.PROGRAM, S.BATCH, S.IS_DISABLED, "
        "       IFNULL(M.HAS_CONTAGIOUS, 'false'), "
        "       IFNULL(R.COURSEID, 'GEN101') " 
        "FROM STUDENT S "
        "LEFT JOIN ROUTINE R ON S.DEPARTMENT = R.PROGRAM AND S.BATCH = R.BATCH " 
        "LEFT JOIN MEDICAL M ON S.REGID = M.REGID "
        "ORDER BY IFNULL(R.COURSEID, 'GEN101') ASC, S.SN ASC;"; 

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(DB, joinQuery.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            string name = (char*)sqlite3_column_text(stmt, 1) ? (char*)sqlite3_column_text(stmt, 1) : "UNKNOWN";
            string reg = (char*)sqlite3_column_text(stmt, 2) ? (char*)sqlite3_column_text(stmt, 2) : "";
            string roll = (char*)sqlite3_column_text(stmt, 3) ? (char*)sqlite3_column_text(stmt, 3) : "0";
            string dept = (char*)sqlite3_column_text(stmt, 4) ? (char*)sqlite3_column_text(stmt, 4) : "";
            string sem = (char*)sqlite3_column_text(stmt, 5) ? (char*)sqlite3_column_text(stmt, 5) : "Sem 1";
            string prog = (char*)sqlite3_column_text(stmt, 6) ? (char*)sqlite3_column_text(stmt, 6) : "BE";
            string batch = (char*)sqlite3_column_text(stmt, 7) ? (char*)sqlite3_column_text(stmt, 7) : "2025";
            string dis = (char*)sqlite3_column_text(stmt, 8) ? (char*)sqlite3_column_text(stmt, 8) : "false";
            string sick = (char*)sqlite3_column_text(stmt, 9) ? (char*)sqlite3_column_text(stmt, 9) : "false";
            string sub = (char*)sqlite3_column_text(stmt, 10) ? (char*)sqlite3_column_text(stmt, 10) : "GEN101";

            if (!reg.empty()) {
                Student s(id, name, reg, roll, dept, sem, prog, batch, dis, sick, sub, "UNKNOWN");
                studentList.push_back(s);
            }
        }
        sqlite3_finalize(stmt);
    }
}

// core Allocation Engine that applies all 14 Rules
void generateAndExportSeatPlan(sqlite3* DB, vector<Student>& studentList) {
    ofstream csvFile("data/Final_Seat_Plan.csv");
    csvFile << "SN,Registration_ID,Roll_No,Name,Department,Semester,Program,Subject_Code,Assigned_Invigilators,Seat_Code\n";

    map<string, vector<Student>> subjectBuckets;
    vector<Student> isolationStudents;
    vector<string> subjectsOrder;
    size_t normalStudentCount = 0;

    // Rule 3 & 4: Separate normal students from isolation/disabled cases
    for (const auto& s : studentList) {
        if (s.getHasContagious() == "true" || s.getIsDisabled() == "true") {
            isolationStudents.push_back(s);
        } else {
            if (subjectBuckets[s.getSubjectCode()].empty()) {
                subjectsOrder.push_back(s.getSubjectCode());
            }
            subjectBuckets[s.getSubjectCode()].push_back(s);
            normalStudentCount++;
        }
    }

    // Rule 1: Capacity Warning Pre-Check
    int totalAvailableSeats = 9 * 3 * 12; // Total seats in Block A to I
    if ((int)normalStudentCount > totalAvailableSeats) {
        cout << "[WARNING] Total students exceed physical room capacity!" << endl;
    }

    // Reset previous seat plan data inside Database table
    sqlite3_exec(DB, "DELETE FROM SEAT_PLAN;", 0, 0, 0);

    // Rule 12 & 13: Calculate required invigilators based on 1:10 ratio
    int requiredInvigilators = max(2, (int)ceil((double)normalStudentCount / 10.0));

    char currentBlock = 'A';
    int subBlocksCount = 3; 
    int maxSeatsPerSubBlock = 12; 

    map<string, size_t> bucketIndices;
    for(auto const& sub : subjectsOrder) {
        bucketIndices[sub] = 0;
    }

    size_t globalSubjectPointer = 0;
    int outputSN = 1; // Sequential Serial Number Counter starting from 1

    bool studentsRemaining = true;
    while (studentsRemaining && currentBlock <= 'I') { 
        studentsRemaining = false;

        for (int r = 1; r <= subBlocksCount; ++r) {
            string subgroupLabel = string(1, currentBlock) + to_string(r);
            if (subjectsOrder.empty()) break;

            // Rule 5, 6, 7, 10: Even/Odd subject picking for side-by-side separation
            string subOdd = subjectsOrder[globalSubjectPointer % subjectsOrder.size()];
            string subEven = subjectsOrder[(globalSubjectPointer + 1) % subjectsOrder.size()];
            globalSubjectPointer++; 

            for (int seat = 1; seat <= maxSeatsPerSubBlock; ++seat) {
                Student* selectedStudent = nullptr;

                if (seat % 2 != 0) { // ODD Seats allocation logic
                    size_t &idx = bucketIndices[subOdd];
                    if (idx < subjectBuckets[subOdd].size()) {
                        selectedStudent = &subjectBuckets[subOdd][idx];
                        idx++;
                        studentsRemaining = true;
                    } else {
                        // Fallback mechanism to fill seats if primary bucket is empty
                        for(auto const& altSub : subjectsOrder) {
                            if(bucketIndices[altSub] < subjectBuckets[altSub].size()) {
                                selectedStudent = &subjectBuckets[altSub][bucketIndices[altSub]];
                                bucketIndices[altSub]++;
                                studentsRemaining = true;
                                break;
                            }
                        }
                    }
                } else { // EVEN Seats allocation logic
                    size_t &idx = bucketIndices[subEven];
                    if (idx < subjectBuckets[subEven].size()) {
                        selectedStudent = &subjectBuckets[subEven][idx];
                        idx++;
                        studentsRemaining = true;
                    } else {
                        // Fallback mechanism to fill seats if primary bucket is empty
                        for(auto const& altSub : subjectsOrder) {
                            if(bucketIndices[altSub] < subjectBuckets[altSub].size()) {
                                selectedStudent = &subjectBuckets[altSub][bucketIndices[altSub]];
                                bucketIndices[altSub]++;
                                studentsRemaining = true;
                                break;
                            }
                        }
                    }
                }

                // Write assigned student row to CSV and Database
                if (selectedStudent != nullptr) {
                    string calculatedSeat = "MH-" + subgroupLabel + "-ST" + to_string(seat);
                    selectedStudent->setSeatCode(calculatedSeat);
                    
                    string primaryTeacher = getAssignedTeacherFromDB(DB, selectedStudent->getSubjectCode());
                    string invigilatorString = "Chief: " + primaryTeacher + " + 1 Asst [Total Required for Hall: " + to_string(requiredInvigilators) + "]";

                    csvFile << outputSN << "," << selectedStudent->getRegNo() << ","
                            << selectedStudent->getRollNo() << "," << selectedStudent->getName() << ","
                            << selectedStudent->getDepartment() << "," << selectedStudent->getSemester() << ","
                            << selectedStudent->getProgram() << "," << selectedStudent->getSubjectCode() << ",\""
                            << invigilatorString << "\"," << selectedStudent->getSeatCode() << "\n";

                    string sql = "INSERT INTO SEAT_PLAN VALUES (" + to_string(outputSN) + ",'" +
                                 selectedStudent->getRegNo() + "','" + selectedStudent->getRollNo() + "','" +
                                 selectedStudent->getName() + "','" + selectedStudent->getDepartment() + "','" +
                                 selectedStudent->getSemester() + "','" + selectedStudent->getProgram() + "','" +
                                 selectedStudent->getSubjectCode() + "','" + invigilatorString + "','" + selectedStudent->getSeatCode() + "');";
                    sqlite3_exec(DB, sql.c_str(), 0, 0, 0);
                    
                    outputSN++; 
                }
            }
        }
        currentBlock++; 
    }

    // Rule 3 & 4: Process medical and physical isolation students into dynamic ISO-ROOM
    for (auto& s : isolationStudents) {
        s.setSeatCode("ISO-ROOM");
        string isolationStaff = "Chief: Medical Team Incharge + 1 Nurse";
        
        csvFile << outputSN << "," << s.getRegNo() << "," << s.getRollNo() << ","
                << s.getName() << "," << s.getDepartment() << "," << s.getSemester() << ","
                << s.getProgram() << "," << s.getSubjectCode() << ",\"" << isolationStaff << "\",ISO-ROOM\n";
        
        string sql = "INSERT INTO SEAT_PLAN VALUES (" + to_string(outputSN) + ",'" + s.getRegNo() + "','" + s.getRollNo() + "','" + s.getName() + "','" + s.getDepartment() + "','" + s.getSemester() + "','" + s.getProgram() + "','" + s.getSubjectCode() + "','" + isolationStaff + "','ISO-ROOM');";
        sqlite3_exec(DB, sql.c_str(), 0, 0, 0);
        
        outputSN++;
    }
    csvFile.close();
}