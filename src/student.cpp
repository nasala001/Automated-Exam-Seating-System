#include "student.hpp"

// Teacher Constructor Implementation
Teacher::Teacher(string cid, int b, string n, string contact) {
    courseId = cid;
    batch = b;
    name = n;
    contactNo = contact;
}

// Student Constructor Implementation
Student::Student(string n, string roll, string reg, string prog, bool disease, bool impaired) {
    name = n;
    rollNo = roll;
    regNo = reg;
    program = prog;
    hasContagiousDisease = disease;
    isPhysicallyImpaired = impaired;
    
    // Conditional Department Allocation logic (DoCSE for CE/AI, DESE for others)
    if (prog == "CE") {
        department = "DoCSE";
    } else {
        department = "DESE";
    }
    
    // Default initializations before seat processing runs
    subjectCode = "N/A";
    venue = "N/A";
    group = "N/A";
    seatNumber = 0;
    seatCode = "N/A";
}