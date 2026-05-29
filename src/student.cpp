#include "student.hpp"
#include <sstream>

using namespace std;

Student::Student() {
    name = "";
    rollNo = "";
    regNo = "";
    program = "";
    venue = "N/A";
    group = "N/A";
    seatNumber = 0;
    seatCode = "N/A";
}

void Student::setBasicInfo(string n, string r, string rg, string p) {
    name = n;
    rollNo = r;
    regNo = rg;
    program = p;
}

string Student::getName() { return name; }
string Student::getRollNo() { return rollNo; }
string Student::getRegNo() { return regNo; }
string Student::getProgram() { return program; }
string Student::getVenue() { return venue; }
string Student::getGroup() { return group; }
int Student::getSeatNumber() { return seatNumber; }
string Student::getSeatCode() { return seatCode; }

// Using 'line' as the name of the incoming string row variable
void breakCsvRow(string line, string words[], int &count) {
    stringstream ss(line);
    string dataToken;
    count = 0;
    
    while (getline(ss, dataToken, ',')) {
        words[count] = dataToken;
        count++;
    }
}