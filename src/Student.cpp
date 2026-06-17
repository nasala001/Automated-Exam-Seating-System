#include "Student.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

// ---------------------------------------------------------------------------
// CSV helper — splits a single line by commas, trims whitespace.
// ---------------------------------------------------------------------------
static vector<string> splitCSVLine(const string &line) {
  vector<string> fields;
  istringstream ss(line);
  string field;
  while (getline(ss, field, ',')) {
    // Trim leading/trailing whitespace
    size_t start = field.find_first_not_of(" \t\r\n");
    size_t end = field.find_last_not_of(" \t\r\n");
    if (start == string::npos)
      fields.push_back("");
    else
      fields.push_back(field.substr(start, end - start + 1));
  }
  return fields;
}

// ---------------------------------------------------------------------------
// parseStudentsFromCSV
// ---------------------------------------------------------------------------
vector<Student> parseStudentsFromCSV(const string &filepath) {
  vector<Student> students;

  ifstream file(filepath);
  if (!file.is_open()) {
    cerr << "  [!] Error: Cannot open file \"" << filepath << "\"\n";
    return students;
  }

  string line;

  // Skip header line
  if (!getline(file, line)) {
    cerr << "  [!] Error: File is empty.\n";
    return students;
  }

  int lineNum = 1;
  while (getline(file, line)) {
    ++lineNum;
    if (line.empty())
      continue;

    vector<string> fields = splitCSVLine(line);
    if (fields.size() < 6) {
      cerr << "  [!] Warning: Line " << lineNum
           << " has fewer than 6 fields, skipping.\n";
      continue;
    }

    Student s;
    s.name = fields[0];
    s.examRollNo = fields[1];
    s.registrationNo = fields[2];
    s.program = fields[3];

    // Parse semester (default to 0 if invalid)
    try {
      s.semester = stoi(fields[4]);
    } catch (...) {
      s.semester = 0;
    }

    s.subject = fields[5];
    students.push_back(s);
  }

  cout << "  Loaded " << students.size() << " students from \"" << filepath
       << "\"\n";
  return students;
}
