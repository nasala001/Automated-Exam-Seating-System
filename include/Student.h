#pragma once

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Student
// Represents one examinee to be assigned a seat.
// ---------------------------------------------------------------------------
struct Student {
    std::string name;           // e.g. "Ram Sharma"
    std::string examRollNo;     // e.g. "CE-2081-001"
    std::string registrationNo; // e.g. "2081-2-03-001"
    std::string program;        // e.g. "CE", "AI", "BIT"
    int         semester;       // e.g. 6
    std::string subject;        // exam subject, e.g. "DAA"
};

// Loads a list of students from a CSV file.
// Expected CSV format (first line is header, skipped):
//   Name,ExamRollNo,RegNo,Program,Semester,Subject
//
// Returns an empty vector and prints an error if the file cannot be opened.
std::vector<Student> parseStudentsFromCSV(const std::string& filepath);
