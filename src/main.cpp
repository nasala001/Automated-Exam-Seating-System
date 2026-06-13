#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "student.hpp"  // Linking our custom blueprint header

using namespace std;

int main() {
    // --------------------------------------------------------------------
    // STAGE 1: LOADING DATA STRUCTURES FROM CSV SIMULATIONS
    // --------------------------------------------------------------------
    vector<Teacher> teacherList;
    teacherList.push_back(Teacher("COMP116", 2025, "Pankaj Kumar", "9818XXXXXX"));
    teacherList.push_back(Teacher("ENGG103", 2025, "Prof. Ram Swarth", "9843XXXXXX"));

    vector<Student> cePool;
    vector<Student> civilPool;

    // Dynamically loading 50 students into respective faculty streams
    for (int i = 1; i <= 50; i++) {
        string reg = "REG" + to_string(100 + i);
        bool isSick = (i == 3 || i == 39);        // Mock Medical exceptions
        bool isDisabled = (i == 5 || i == 13);    // Mock Locomotive exceptions

        if (i % 2 != 0) {
            // CE Stream Setup
            Student s("CE Candidate " + to_string(i), to_string((i+1)/2), reg, "CE", isSick, isDisabled);
            s.subjectCode = "COMP116"; 
            cePool.push_back(s);
        } else {
            // Civil Stream Setup
            Student s("Civil Candidate " + to_string(i), to_string(i/2), reg, "Civil", isSick, isDisabled);
            s.subjectCode = "ENGG103"; 
            civilPool.push_back(s);
        }
    }

    // Pipeline vectors for separation
    vector<Student> normalHall;
    vector<Student> accessibleRooms;
    vector<Student> isolationRooms;

    // Interleave sorting: Mixing CE and Civil palopalo side-by-side
    size_t maxSize = max(cePool.size(), civilPool.size());
    for (size_t i = 0; i < maxSize; i++) {
        if (i < cePool.size()) {
            if (cePool[i].hasContagiousDisease) isolationRooms.push_back(cePool[i]);
            else if (cePool[i].isPhysicallyImpaired) accessibleRooms.push_back(cePool[i]);
            else normalHall.push_back(cePool[i]);
        }
        if (i < civilPool.size()) {
            if (civilPool[i].hasContagiousDisease) isolationRooms.push_back(civilPool[i]);
            else if (civilPool[i].isPhysicallyImpaired) accessibleRooms.push_back(civilPool[i]);
            else normalHall.push_back(civilPool[i]);
        }
    }

    // --------------------------------------------------------------------
    // STAGE 2: MULTIPURPOSE HALL INTERLEAVED SEATING ENGINE
    // --------------------------------------------------------------------
    const int SEAT_LIMIT_PER_ROW = 10; 
    char mainBlock = 'A'; 
    int rowGroup = 1;
    int localSeatIndex = 1;

    cout << "=========================================================================\n";
    cout << "      KATHMANDU UNIVERSITY AESMS PANEL - MODULAR ARCHITECTURE EXECUTION \n";
    cout << "=========================================================================\n\n";

    for (size_t i = 0; i < normalHall.size(); i++) {
        if (localSeatIndex > SEAT_LIMIT_PER_ROW) {
            rowGroup++;
            localSeatIndex = 1;
            if (rowGroup > 3) {
                mainBlock++;
                rowGroup = 1;
            }
        }

        normalHall[i].venue = "Multipurpose Hall Main Complex";
        normalHall[i].group = string(1, mainBlock) + to_string(rowGroup);
        normalHall[i].seatNumber = localSeatIndex;
        normalHall[i].seatCode = normalHall[i].group + "-S" + (localSeatIndex < 10 ? "0" : "") + to_string(localSeatIndex);

        string auditStatus = "COMPLIANT (INTERLEAVED)";
        if (i > 0 && normalHall[i].program == normalHall[i-1].program) {
            auditStatus = "WARNING: SAME PROGRAM ADJACENT FAULT";
        }

        cout << "RegID: " << normalHall[i].regNo 
             << " | Dept: " << normalHall[i].department
             << " | Faculty: " << normalHall[i].program 
             << " | Course: " << normalHall[i].subjectCode 
             << " | Seat Code: " << normalHall[i].seatCode 
             << " | Audit: " << auditStatus << endl;

        localSeatIndex++;
    }

    // --------------------------------------------------------------------
    // STAGE 3: BLOCK 9 ACCESSIBILITY ROUTING (ROOM 202/203)
    // --------------------------------------------------------------------
    cout << "\n-------------------------------------------------------------------------\n";
    cout << "      ACCESSIBILITY DIVISION LOGS - BLOCK 9 LOWER FLOORS (ROOM 202/203)  \n";
    cout << "-------------------------------------------------------------------------\n";
    string accessRoomsPool[] = {"Room 202", "Room 203"};
    for (size_t i = 0; i < accessibleRooms.size(); i++) {
        string currentRoom = accessRoomsPool[i % 2];
        accessibleRooms[i].venue = "Block 9 (" + currentRoom + ")";
        accessibleRooms[i].seatCode = "B9-" + currentRoom.substr(5) + "-S0" + to_string(i + 1);
        
        cout << "[ACCESSIBLE AREA] RegID: " << accessibleRooms[i].regNo 
             << " | Branch: " << accessibleRooms[i].program 
             << " | Room Target: " << accessibleRooms[i].venue 
             << " | Seat Code: " << accessibleRooms[i].seatCode << endl;
    }

    // --------------------------------------------------------------------
    // STAGE 4: BLOCK 9 ISOLATION ROUTING (ROOM 402/404)
    // --------------------------------------------------------------------
    cout << "\n-------------------------------------------------------------------------\n";
    cout << "      CRITICAL BIOSAFETY CHANNELS - BLOCK 9 ISOLATION FLOORS (ROOM 402/404)\n";
    cout << "-------------------------------------------------------------------------\n";
    string isoRoomsPool[] = {"Room 402", "Room 404"};
    for (size_t i = 0; i < isolationRooms.size(); i++) {
        string currentRoom = isoRoomsPool[i % 2];
        isolationRooms[i].venue = "Block 9 (" + currentRoom + " - Isolation)";
        isolationRooms[i].seatCode = "B9-" + currentRoom.substr(5) + "-ISO-S0" + to_string(i + 1);
        
        cout << "[QUARANTINE ENFORCED] RegID: " << isolationRooms[i].regNo 
             << " | Branch: " << isolationRooms[i].program 
             << " | Secure Enclosure: " << isolationRooms[i].venue 
             << " | Isolation Seat: " << isolationRooms[i].seatCode << endl;
    }

    return 0;
}