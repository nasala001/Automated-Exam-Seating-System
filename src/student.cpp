#include "student.hpp"
#include <sstream>

using namespace std;

// Default allocation structures tracking memory variables clear
Student::Student() {
    name = "";
    rollNo = "";
    regNo = "";
    program = "";
    department = "DoCSE";
    subject = "N/A";
    hasContagiousDisease = false;
    isPhysicallyImpaired = false;
    
    venue = "N/A";
    group = "N/A";
    seatNumber = 0;
    seatCode = "N/A";
}

void Student::setFullDetails(string n, string r, string rg, string p, string dept, string sub, bool contagious, bool impaired) {
    name = n;
    rollNo = r;
    regNo = rg;
    program = p;
    department = dept;
    subject = sub;
    hasContagiousDisease = contagious;
    isPhysicallyImpaired = impaired;
}

void Student::setBasicInfo(string n, string r, string rg, string p) {
    name = n;
    rollNo = r;
    regNo = rg;
    program = p;
}

void Student::setVenue(string v) { venue = v; }
void Student::setGroup(string g) { group = g; }
void Student::setSeatNumber(int sNum) { seatNumber = sNum; }
void Student::setSeatCode(string sCode) { seatCode = sCode; }

string Student::getName() { return name; }
string Student::getRollNo() { return rollNo; }
string Student::getRegNo() { return regNo; }
string Student::getProgram() { return program; }
string Student::getDepartment() { return department; }
string Student::getSubject() { return subject; }
bool Student::getHasContagiousDisease() { return hasContagiousDisease; }
bool Student::getIsPhysicallyImpaired() { return isPhysicallyImpaired; }

string Student::getVenue() { return venue; }
string Student::getGroup() { return group; }
int Student::getSeatNumber() { return seatNumber; }
string Student::getSeatCode() { return seatCode; }

// Dynamic stream parsing splitter engine without syntax issues
void breakCsvRow(string line, string words[], int &count) {
    stringstream ss(line);
    string dataToken;
    count = 0;
    
    while (getline(ss, dataToken, ',')) {
        size_t start = dataToken.find_first_not_of(" \t\r\n");
        size_t end = dataToken.find_last_not_of(" \t\r\n");
        if (start != string::npos && end != string::npos) {
            words[count] = dataToken.substr(start, (end - start + 1));
        } else {
            words[count] = "";
        }
        count++;
    }
}