#include <iostream>
#include <fstream>
#include "student.hpp"

using namespace std;

int main() {
    cout << "=== Exam Seating System Module 3 Prototype ===" << endl;

    Student studentDatabase[100];
    int totalStudents = 0;

    string filename = "main.csv";

    ifstream myfile(filename);

    if (myfile.is_open() == false) {
        cout << "[Warning] File vettiyena! Empty data container usage." << endl;
        return 0;
    }

    string line;
    getline(myfile, line); // Skipping the header line (Name,RollNo...)

    while (getline(myfile, line)) {
        if (line == "") {
            continue; 
        }

        string splitPieces[10];
        int totalPieces = 0;
        
        breakCsvRow(line, splitPieces, totalPieces);

        if (totalPieces >= 4) {
            studentDatabase[totalStudents].setBasicInfo(
                splitPieces[0], 
                splitPieces[1], 
                splitPieces[2], 
                splitPieces[3]  
            );
            totalStudents++;
        }
    }
    myfile.close();

    cout << "Loaded records count from sheet: " << totalStudents << endl;
 
    ofstream outClassFile("CE_Seating.csv");
    if (outClassFile.is_open()) {
        outClassFile << "Student Name,Exam Roll No.,Registration No.,Program,Venue,Group,Seat Number,Seat Code\n";
        
        for (int i = 0; i < totalStudents; i++) {
            if (studentDatabase[i].getProgram() == "CE") {
                outClassFile << studentDatabase[i].getName() << ","
                             << studentDatabase[i].getRollNo() << ","
                             << studentDatabase[i].getRegNo() << ","
                             << studentDatabase[i].getProgram() << ","
                             << studentDatabase[i].getVenue() << ","
                             << studentDatabase[i].getGroup() << ","
                             << studentDatabase[i].getSeatNumber() << ","
                             << studentDatabase[i].getSeatCode() << "\n";
            }
        }
        outClassFile.close();
    }

    // TODO: Aba arkko hafta yo kam garna baki cha. 
    // Supervisor sir le college ko real data diyepachi, roll number wise sort garna parcha.
    // Tyo belama normal Bubble Sort algorithm use garera loop chalaune ho, aile lai skip gareko chu.
    cout << "\n[ALERT] Master sorted list export status: PENDING" << endl;
    cout << "Sir le data diyepachi logic implement garchu arkko hafta." << endl;

    return 0;
}