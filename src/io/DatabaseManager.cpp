#include "io/DatabaseManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<Student> DatabaseManager::loadStudentsFromCSV(const std::string& filePath) {
    std::vector<Student> students;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return students;
    }

    std::string line;
    // Skip header
    std::getline(file, line);
    
    // Expected format: name,registrationNo,rollNo,program,department,subject,semester,isPhysicallyImpaired
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string name, regNo, rollNo, program, department, subject, semesterStr, impairedStr;

        std::getline(ss, name, ',');
        std::getline(ss, regNo, ',');
        std::getline(ss, rollNo, ',');
        std::getline(ss, program, ',');
        std::getline(ss, department, ',');
        std::getline(ss, subject, ',');
        std::getline(ss, semesterStr, ',');
        std::getline(ss, impairedStr, ',');

        Student s;
        s.name = name;
        s.registrationNo = regNo;
        s.rollNo = rollNo;
        s.program = program;
        s.department = department;
        s.subject = subject;
        try { s.semester = std::stoi(semesterStr); } catch(...) { s.semester = 1; }
        s.isPhysicallyImpaired = (impairedStr == "true" || impairedStr == "1" || impairedStr == "Yes");
        
        students.push_back(s);
    }
    return students;
}
