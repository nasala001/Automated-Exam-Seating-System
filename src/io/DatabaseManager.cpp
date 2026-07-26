#include "io/DatabaseManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace shresh {

// Helper to trim whitespace
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// RFC-4180 compliant line split
static std::vector<std::string> splitCSVLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            tokens.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    tokens.push_back(trim(current));
    return tokens;
}

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
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<std::string> tokens = splitCSVLine(line);
        if (tokens.size() < 8) continue; // must have at least 8 columns

        Student s;
        s.name = tokens[0];
        s.registrationNo = tokens[1];
        s.rollNo = tokens[2];
        s.program = tokens[3];

        std::string semesterStr, impairedStr, contagiousStr = "false";

        if (tokens.size() == 8) {
            s.section = "";
            s.department = tokens[4];
            s.subject = tokens[5];
            semesterStr = tokens[6];
            impairedStr = tokens[7];
        } else {
            s.section = tokens[4];
            s.department = tokens[5];
            s.subject = tokens[6];
            semesterStr = tokens[7];
            impairedStr = tokens[8];
            if (tokens.size() > 9) {
                contagiousStr = tokens[9];
            }
        }

        try { s.semester = std::stoi(semesterStr); } catch(...) { s.semester = 1; }
        s.isPhysicallyImpaired = (impairedStr == "true" || impairedStr == "1" || impairedStr == "Yes" || impairedStr == "true");
        s.hasContagiousDisease = (contagiousStr == "true" || contagiousStr == "1" || contagiousStr == "Yes");
        
        students.push_back(s);
    }
    return students;
}

} // namespace shresh
