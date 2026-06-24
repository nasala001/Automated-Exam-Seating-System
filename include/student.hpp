#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "../database/sqlite3.h"
#include <string>
#include <vector>

class Student {
private:
    std::string name;
    std::string regNo;
    std::string rollNo;
    std::string program;
    std::string batch;
    std::string isDisabled;
    std::string hasContagious; // From medical.csv
    std::string subjectCode;   // From routine.csv
    std::string teacherName;   // From teacher.csv (Sir's Requirement)
    std::string seatCode;      // Generated dynamic seat format

public:
    // Constructor
    Student(std::string n, std::string reg, std::string roll, std::string prog, 
            std::string b, std::string dis, std::string sick = "false", 
            std::string sub = "UNKNOWN", std::string teach = "UNKNOWN");

    // Getters and Setters
    std::string getName() const { return name; }
    std::string getRegNo() const { return regNo; }
    std::string getRollNo() const { return rollNo; }
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