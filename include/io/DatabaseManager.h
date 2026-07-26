#pragma once

#include <string>
#include <vector>
#include "../../shresh_validation/DataStructures.h"

namespace shresh {

class DatabaseManager {
public:
    // Parses the Student_Info.csv file and returns a list of Students defined in DataStructures.h
    static std::vector<Student> loadStudentsFromCSV(const std::string& filePath);
};

} // namespace shresh

