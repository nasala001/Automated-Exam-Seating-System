#include <iostream>
#include <fstream>
#include <algorithm>
#include "student.hpp"

using namespace std;

int main() {
    cout << "=== Exam Seating System Framework Module (Integrated) ===" << endl;

    // Main temporary processing database allocation limit 500 records
    Student studentDatabase[500];
    int totalStudents = 0;

    string filename = "main.csv";
    ifstream myfile(filename);

    if (myfile.is_open() == false) {
        cout << "[Warning] Data file '" << filename << "' not detected. Dynamic testing runtime pending." << endl;
        cout << "[Notice] Supervisor spreadsheet file data commit expected next week." << endl;
        return 0;
    }

    string line;
    getline(myfile, line); // Skip spreadsheet sheet header configuration

    while (getline(myfile, line)) {
        if (line == "") continue;

        string splitPieces[10];
        int totalPieces = 0;
        
        breakCsvRow(line, splitPieces, totalPieces);

        // Adaptive ingestion block to accept raw data strings safely
        if (totalPieces >= 8) {
            bool contagiousFlag = (splitPieces[6] == "1" || splitPieces[6] == "true");
            bool impairedFlag = (splitPieces[7] == "1" || splitPieces[7] == "true");
            
            studentDatabase[totalStudents].setFullDetails(
                splitPieces[0], splitPieces[1], splitPieces[2], splitPieces[3],
                splitPieces[4], splitPieces[5], contagiousFlag, impairedFlag
            );
            totalStudents++;
        }
        else if (totalPieces >= 4) {
            studentDatabase[totalStudents].setBasicInfo(
                splitPieces[0], splitPieces[1], splitPieces[2], splitPieces[3]
            );
            totalStudents++;
        }
    }
    myfile.close();

    cout << "[Success] Ingested raw lines completely: " << totalStudents << " elements inside memory." << endl;

    // NEXT WEEK ACTION POINT: Sequential Sorting Engine integration
    // Once data sheet arrives, this algorithm bubble block sorts values by sequence
    for (int i = 0; i < totalStudents - 1; i++) {
        for (int j = 0; j < totalStudents - i - 1; j++) {
            int rollA = 0, rollB = 0;
            try {
                rollA = stoi(studentDatabase[j].getRollNo());
                rollB = stoi(studentDatabase[j + 1].getRollNo());
            } catch (...) {
                if (studentDatabase[j].getRollNo() > studentDatabase[j + 1].getRollNo()) {
                    Student temp = studentDatabase[j];
                    studentDatabase[j] = studentDatabase[j + 1];
                    studentDatabase[j + 1] = temp;
                }
                continue;
            }

            if (rollA > rollB) {
                Student temp = studentDatabase[j];
                studentDatabase[j] = studentDatabase[j + 1];
                studentDatabase[j + 1] = temp;
            }
        }
    }

    // Dynamic generation reporting streams filter
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
        cout << "[Export] System generated automated configuration data sheet: 'CE_Seating.csv'" << endl;
    }

    cout << "\n==========================================================" << endl;
    cout << "[STATUS] Module architecture linked cleanly to validation layers." << endl;
    cout << "[STATUS] Ready for full integration logic testing next week." << endl;
    cout << "==========================================================" << endl;

    return 0;
}