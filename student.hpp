#ifndef STUDENT_HPP
#define STUDENT_HPP

#include <string>

using namespace std;

class Student {
private:
    // Sathi ko structural data coordinates match gareko fields
    string name;
    string rollNo;          
    string regNo;  
    string program;         
    string department;      
    string subject;         
    
    bool hasContagiousDisease;
    bool isPhysicallyImpaired;

    // Seating tracker configuration parameters
    string venue;
    string group;
    int seatNumber;
    string seatCode;

public:
    Student();
    
    // Core parameters injection handler
    void setFullDetails(string n, string r, string rg, string p, string dept, string sub, bool contagious, bool impaired);
    void setBasicInfo(string n, string r, string rg, string p); 
    
    void setVenue(string v);
    void setGroup(string g);
    void setSeatNumber(int sNum);
    void setSeatCode(string sCode);

    // Dynamic data access gates (Getters)
    string getName();
    string getRollNo();
    string getRegNo();
    string getProgram();
    string getDepartment();
    string getSubject();
    bool getHasContagiousDisease();
    bool getIsPhysicallyImpaired();
    
    string getVenue();
    string getGroup();
    int getSeatNumber();
    string getSeatCode();
};

// Comma tracking utility processor definition
void breakCsvRow(string line, string words[], int &count);

#endif // STUDENT_HPP