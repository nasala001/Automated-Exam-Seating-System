#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "../database/sqlite3.h"
#include <string>
#include <vector>

class Student {
private:
    int sn;                     // Primary Identifier Key
    std::string name;
    std::string regNo;          
    std::string rollNo;
    std::string department;     
    std::string semester;       
    std::string program;        
    std::string batch;
    std::string isDisabled;
    std::string hasContagious;  // Joined from Medical logs
    std::string subjectCode;    // Joined from Exam routines
    std::string teacherName;    // Joined from Teacher Info list
    std::string seatCode;       // Dynamic output field tracker

public:
    // Core class constructor interface
    Student(int id, std::string n, std::string reg, std::string roll, std::string dept, 
            std::string sem, std::string prog, std::string b, std::string dis, 
            std::string sick = "false", std::string sub = "UNKNOWN", std::string teach = "UNKNOWN");

    // Interface Accessors 
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

// Execution Pipe Function Signatures
bool runDatabaseMigrationEngine(sqlite3* DB);
void loadLiveRecordsIntoVectors(sqlite3* DB, std::vector<Student>& studentList);
void generateAndExportSeatPlan(sqlite3* DB, std::vector<Student>& studentList);

#endif