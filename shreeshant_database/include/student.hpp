#ifndef STUDENT_HPP
#define STUDENT_HPP

#include <string>

using namespace std;

// Teacher Blueprint Mapping
class Teacher {
public:
    string courseId;    
    int batch;
    string name;
    string contactNo;

    Teacher(string cid, int b, string n, string contact);
};

// Student Blueprint Mapping with 12 Core Parameters
class Student {
public:
    string name;
    string rollNo;
    string regNo;
    string program;        // CE or Civil
    string department;     // DoCSE or DESE
    string subjectCode;    // Mapped via the dynamic parallel routine
    bool hasContagiousDisease;
    bool isPhysicallyImpaired;
    
    // Core structural variables used for routing and printing
    string venue;
    string group;
    int seatNumber;
    string seatCode;

    // Constructor Declaration
    Student(string n, string roll, string reg, string prog, bool disease, bool impaired);
};

#endif // STUDENT_HPP