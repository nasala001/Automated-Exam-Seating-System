#include "../include/student.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

Student::Student(int id, string n, string reg, string roll, string dept, string sem, string prog, string b, string dis, string sick, string sub, string teach)
    : sn(id), name(n), regNo(reg), rollNo(roll), department(dept), semester(sem), program(prog), batch(b), isDisabled(dis), hasContagious(sick), subjectCode(sub), teacherName(teach), seatCode("UNASSIGNED") {}

// --- यहाँ नयाँ शिक्षक एसाइन गर्ने लोजिक हालिएको छ ---
string getAssignedTeacher(const string& seatCode) {
    if (seatCode == "ISO-ROOM") {
        return "Medical Team Incharge";
    }

    // सिट कोडको सुरुवाती ५ क्यारेक्टर लिने (जस्तै: "MH-A1")
    string subGroup = seatCode.substr(0, 5);

    // प्रत्येक सब-ग्रुप अनुसार फरक-फरक शिक्षकहरूको म्यापिङ
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
    
    return "Pankaj Kumar"; // यदि कुनै पनि मिलेन भने डिफल्ट
}

void loadLiveRecordsIntoVectors(sqlite3* DB, vector<Student>& studentList) {
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

void generateAndExportSeatPlan(sqlite3* DB, vector<Student>& studentList) {
    ofstream csvFile("data/Final_Seat_Plan.csv");
    
    csvFile << "SN,Registration_ID,Roll_No,Name,Department,Semester,Program,Subject_Code,Assigned_Teacher,Seat_Code\n";

    map<string, vector<Student>> subjectBuckets;
    vector<Student> isolationStudents;
    vector<string> subjectsOrder;

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

    char currentBlock = 'A';
    int maxSeatsPerRow = 12; 
    
    map<string, size_t> bucketIndices;
    for(auto const& sub : subjectsOrder) {
        bucketIndices[sub] = 0;
    }

    bool studentsRemaining = true;

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

                    // --- यहाँ सिट कोड अनुसार डाइनामिकली शिक्षकको नाम सेट गरिन्छ ---
                    string dynamicTeacher = getAssignedTeacher(calculatedSeat);

                    csvFile << s.getSN() << "," << s.getRegNo() << "," << s.getRollNo() << ","
                            << s.getName() << "," << s.getDepartment() << "," << s.getSemester() << ","
                            << s.getProgram() << "," << s.getSubjectCode() << "," << dynamicTeacher << ","
                            << s.getSeatCode() << "\n";

                    string sql = "INSERT INTO SEAT_PLAN VALUES (" + to_string(s.getSN()) + ", '" + s.getRegNo() + "', '" 
                                 + s.getRollNo() + "', '" + s.getName() + "', '" + s.getDepartment() + "', '" 
                                 + s.getSemester() + "', '" + s.getProgram() + "', '" + s.getSubjectCode() + "', '" 
                                 + dynamicTeacher + "', '" + s.getSeatCode() + "');";
                    sqlite3_exec(DB, sql.c_str(), 0, 0, 0);

                    seatCounter++;
                    startIdx++;
                }
            }
        }

        if (studentsRemaining) {
            currentBlock++;
        }
    }

    // Process Isolation Records
    for (auto& s : isolationStudents) {
        s.setSeatCode("ISO-ROOM");
        
        // --- आइसोलेसनका लागि पनि डाइनामिकली शिक्षक सेट गरेको ---
        string dynamicTeacher = getAssignedTeacher("ISO-ROOM");

        csvFile << s.getSN() << "," << s.getRegNo() << "," << s.getRollNo() << ","
                << s.getName() << "," << s.getDepartment() << "," << s.getSemester() << ","
                << s.getProgram() << "," << s.getSubjectCode() << "," << dynamicTeacher << ","
                << s.getSeatCode() << "\n";

        string sql = "INSERT INTO SEAT_PLAN VALUES (" + to_string(s.getSN()) + ", '" + s.getRegNo() + "', '" 
                     + s.getRollNo() + "', '" + s.getName() + "', '" + s.getDepartment() + "', '" 
                     + s.getSemester() + "', '" + s.getProgram() + "', '" + s.getSubjectCode() + "', '" 
                     + dynamicTeacher + "', '" + s.getSeatCode() + "');";
        sqlite3_exec(DB, sql.c_str(), 0, 0, 0);
    }

    csvFile.close();
    cout << "[SUCCESS] Fixed Seat Plan generated! Total rows processed: " << studentList.size() << endl;
}