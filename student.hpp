#ifndef STUDENT_HPP
#define STUDENT_HPP

#include <string>

using namespace std;

class Student {
private:
    string name;
    string rollNo;
    string regNo;
    string program;
    
    string venue;
    string group;
    int seatNumber;
    string seatCode;

public:
    Student();
    void setBasicInfo(string n, string r, string rg, string p);
    
    string getName();
    string getRollNo();
    string getRegNo();
    string getProgram();
    string getVenue();
    string getGroup();
    int getSeatNumber();
    string getSeatCode();
};

// Declaring our helper function using the generic name parameter
void breakCsvRow(string line, string words[], int &count);

#endif