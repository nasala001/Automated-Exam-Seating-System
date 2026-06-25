#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "../database/sqlite3.h"
#include <string>
#include <vector>

class Student {
private:
    int sn;                     // Primary Key (ID is SN)
    std::string name;
    std::string regNo;          // KU Prefix
    std::string rollNo;
    std::string department;     // New Field
    std::string semester;       // New Field
    std::string program;        // New Field
    std::string batch;
    std::string isDisabled;
    std::string hasContagious;  // From Medical_Log.csv
    std::string subjectCode;    // From Exam_Routine.csv
    std::string teacherName;    // From Teacher_Info.csv
    std::string seatCode;       // Generated dynamic seat format

public:
    // Constructor Updated with New Fields
    Student(int id, std::string n, std::string reg, std::string roll, std::string dept, 
            std::string sem, std::string prog, std::string b, std::string dis, 
            std::string sick = "false", std::string sub = "UNKNOWN", std::string teach = "UNKNOWN");

    // Getters and Setters
    int getSN() const { return sn; }
    std::string getName() const { return name; }
    std::string getRegNo() const { return regNo; }
    std::string getRollNo() const { return rollNo; }
    std::string getDepartment() const { return department; }
    std::string getSemester() const { return semester; }
    std::string getProgram() const { return program; }
    std::string getBatch() const { return batch; }
    std::string getIsDisabled() const { return isDisabled; }
    std::string getHasContagious() const { return hasContagious; }
    std::string getSubjectCode() const { return subjectCode; }
    std::string getTeacherName() const { return teacherName; }
    std::string getSeatCode() const { return seatCode; }
    
    void setSeatCode(std::string code) { seatCode = code; }
};

// Core Prototypes
bool runDatabaseMigrationEngine(sqlite3* DB);
void loadLiveRecordsIntoVectors(sqlite3* DB, std::vector<Student>& studentList);
void generateAndExportSeatPlan(sqlite3* DB, std::vector<Student>& studentList);

#endif