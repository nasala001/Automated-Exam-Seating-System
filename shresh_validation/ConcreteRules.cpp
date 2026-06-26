#include "ConcreteRules.h"
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <string>
#include <cctype>

// Helper to parse integer from roll number
static int parseRollNumber(const std::string& rollNo) {
    std::string numStr;
    for (char c : rollNo) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            numStr += c;
        }
    }
    if (numStr.empty()) {
        return -99999; // Sentinel for non-numeric roll
    }
    try {
        return std::stoi(numStr);
    } catch (...) {
        return -99999;
    }
}

// Helper to extract batch/admission year from registration number
static std::string extractBatchYear(const std::string& regNo) {
    auto pos = regNo.find_last_of('-');
    if (pos != std::string::npos && pos + 1 < regNo.size()) {
        return regNo.substr(pos + 1);
    }
    // Fallback: search for last contiguous digit sequence of length 2 or 4
    std::string year;
    for (auto it = regNo.rbegin(); it != regNo.rend(); ++it) {
        if (std::isdigit(static_cast<unsigned char>(*it))) {
            year = *it + year;
        } else if (!year.empty()) {
            break;
        }
    }
    return year;
}

// Helper to count total students in a SeatPlan
static int countTotalStudents(const SeatPlan& seatPlan) {
    int count = 0;
    for (const auto& row : seatPlan.grid) {
        for (const auto& seat : row) {
            if (seat != nullptr) {
                count++;
            }
        }
    }
    return count;
}

// ==========================================
// Rule 1: Room Capacity Rule
// ==========================================
bool RoomCapacityRule::validate(const SeatPlan& seatPlan) {
    int studentCount = countTotalStudents(seatPlan);
    if (studentCount > seatPlan.room.capacity) {
        warning_ = "Room Capacity Exceeded: Room capacity is " + std::to_string(seatPlan.room.capacity) + 
                   " but " + std::to_string(studentCount) + " students were assigned.";
        return false;
    }
    warning_ = "";
    return true;
}

// ==========================================
// Rule 2: One Student - One Seat Rule
// ==========================================
bool OneStudentOneSeatRule::validate(const SeatPlan& seatPlan) {
    std::unordered_set<std::string> seenRolls;
    std::unordered_set<std::string> seenRegs;
    std::vector<std::string> duplicateRolls;

    for (int r = 0; r < seatPlan.room.rows; ++r) {
        for (int c = 0; c < seatPlan.room.columns; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& seat = seatPlan.grid[r][c];
                if (seat != nullptr) {
                    const auto& student = *seat;
                    if (!student.rollNo.empty()) {
                        if (!seenRolls.insert(student.rollNo).second) {
                            duplicateRolls.push_back(student.rollNo);
                        }
                    }
                    if (!student.registrationNo.empty()) {
                        seenRegs.insert(student.registrationNo);
                    }
                }
            }
        }
    }

    if (!duplicateRolls.empty()) {
        warning_ = "Duplicate Student Placement: The following roll numbers are assigned to multiple seats: ";
        for (size_t i = 0; i < duplicateRolls.size(); ++i) {
            warning_ += duplicateRolls[i] + (i + 1 < duplicateRolls.size() ? ", " : "");
        }
        return false;
    }
    warning_ = "";
    return true;
}

// ==========================================
// Rule 3: Contagious Disease Isolation Rule
// ==========================================
bool ContagiousDiseaseIsolationRule::validate(const SeatPlan& seatPlan) {
    bool hasViolation = false;
    warning_ = "";

    int rows = seatPlan.room.rows;
    int cols = seatPlan.room.columns;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& seat = seatPlan.grid[r][c];
                if (seat != nullptr && seat->hasContagiousDisease) {
                    // Check all 8 surrounding positions
                    for (int dr = -1; dr <= 1; ++dr) {
                        for (int dc = -1; dc <= 1; ++dc) {
                            if (dr == 0 && dc == 0) continue;
                            int nr = r + dr;
                            int nc = c + dc;
                            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                                if (nr < static_cast<int>(seatPlan.grid.size()) && nc < static_cast<int>(seatPlan.grid[nr].size())) {
                                    const auto& adjSeat = seatPlan.grid[nr][nc];
                                    if (adjSeat != nullptr) {
                                        hasViolation = true;
                                        warning_ += "Infection Isolation Violation: Infected student " + seat->name + 
                                                    " (Roll: " + seat->rollNo + ") at (" + std::to_string(r) + "," + std::to_string(c) + 
                                                    ") is seated adjacent to " + adjSeat->name + " (Roll: " + adjSeat->rollNo + 
                                                    ") at (" + std::to_string(nr) + "," + std::to_string(nc) + "). ";
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return !hasViolation;
}

// ==========================================
// Rule 4: Physical Disability and Amanuensis Rule
// ==========================================
bool PhysicalDisabilityRule::validate(const SeatPlan& seatPlan) {
    bool hasViolation = false;
    warning_ = "";
    int rows = seatPlan.room.rows;
    int cols = seatPlan.room.columns;
    int disabledCount = 0;
    int totalStudents = 0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& seat = seatPlan.grid[r][c];
                if (seat != nullptr) {
                    totalStudents++;
                    if (seat->isPhysicallyImpaired) {
                        disabledCount++;
                        // Disabled student should be in front row (row 0) for easy access
                        if (r != 0) {
                            hasViolation = true;
                            warning_ += "Accessibility Violation: Physically impaired student " + seat->name + 
                                        " (Roll: " + seat->rollNo + ") is seated in Row " + std::to_string(r) + 
                                        " instead of the accessible Front Row (Row 0). ";
                        }
                    }
                }
            }
        }
    }

    // Adjust room capacity for assistants (each disabled student requires an assistant taking space)
    int adjustedNeededCapacity = totalStudents + disabledCount;
    if (adjustedNeededCapacity > seatPlan.room.capacity) {
        hasViolation = true;
        warning_ += "Disability Capacity Violation: Room capacity is " + std::to_string(seatPlan.room.capacity) + 
                    ", but total seats needed including amanuenses is " + std::to_string(adjustedNeededCapacity) + 
                    " (" + std::to_string(totalStudents) + " students + " + std::to_string(disabledCount) + " assistants).";
    }

    return !hasViolation;
}

// ==========================================
// Rule 5: Same Program Horizontal Separation Rule
// ==========================================
bool SameProgramHorizontalRule::validate(const SeatPlan& seatPlan) {
    bool hasViolation = false;
    warning_ = "";
    int rows = seatPlan.room.rows;
    int cols = seatPlan.room.columns;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols - 1; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c + 1 < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& s1 = seatPlan.grid[r][c];
                const auto& s2 = seatPlan.grid[r][c + 1];
                if (s1 != nullptr && s2 != nullptr) {
                    if (s1->program == s2->program) {
                        hasViolation = true;
                        warning_ += "Same Program Horizontal Violation: Student " + s1->name + " (Roll: " + s1->rollNo + 
                                    ") and student " + s2->name + " (Roll: " + s2->rollNo + ") from same program '" + 
                                    s1->program + "' are seated next to each other at Row " + std::to_string(r) + 
                                    ", Seats " + std::to_string(c) + " and " + std::to_string(c + 1) + ". ";
                    }
                }
            }
        }
    }
    return !hasViolation;
}

// ==========================================
// Rule 6: Same Class/Semester Separation Rule
// ==========================================
bool SameSemesterSeparationRule::validate(const SeatPlan& seatPlan) {
    bool hasViolation = false;
    warning_ = "";
    int rows = seatPlan.room.rows;
    int cols = seatPlan.room.columns;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols - 1; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c + 1 < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& s1 = seatPlan.grid[r][c];
                const auto& s2 = seatPlan.grid[r][c + 1];
                if (s1 != nullptr && s2 != nullptr) {
                    std::string batch1 = extractBatchYear(s1->registrationNo);
                    std::string batch2 = extractBatchYear(s2->registrationNo);
                    // Same class / semester is characterized by same program and same entry year/batch
                    if (s1->program == s2->program && batch1 == batch2) {
                        hasViolation = true;
                        warning_ += "Same Class/Semester Horizontal Violation: " + s1->name + " and " + s2->name + 
                                    " (both " + s1->program + ", Batch: " + batch1 + ") are seated side-by-side at Row " + 
                                    std::to_string(r) + ", Seats " + std::to_string(c) + " and " + std::to_string(c+1) + ". ";
                    }
                }
            }
        }
    }
    return !hasViolation;
}

// ==========================================
// Rule 7: Consecutive Roll Number Horizontal Separation Rule
// ==========================================
bool ConsecutiveRollSeparationRule::validate(const SeatPlan& seatPlan) {
    bool hasViolation = false;
    warning_ = "";
    int rows = seatPlan.room.rows;
    int cols = seatPlan.room.columns;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols - 1; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c + 1 < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& s1 = seatPlan.grid[r][c];
                const auto& s2 = seatPlan.grid[r][c + 1];
                if (s1 != nullptr && s2 != nullptr) {
                    int roll1 = parseRollNumber(s1->rollNo);
                    int roll2 = parseRollNumber(s2->rollNo);
                    // Check if numeric rolls are consecutive (absolute difference is 1)
                    if (roll1 != -99999 && roll2 != -99999 && std::abs(roll1 - roll2) == 1) {
                        hasViolation = true;
                        warning_ += "Consecutive Roll Horizontal Violation: " + s1->name + " (Roll: " + s1->rollNo + 
                                    ") and " + s2->name + " (Roll: " + s2->rollNo + ") are consecutive and seated next to each other at Row " + 
                                    std::to_string(r) + ", Seats " + std::to_string(c) + " and " + std::to_string(c+1) + ". ";
                    }
                }
            }
        }
    }
    return !hasViolation;
}

// ==========================================
// Rule 8: Same Program Vertical Arrangement Preference
// ==========================================
bool SameProgramVerticalPreferenceRule::validate(const SeatPlan& seatPlan) {
    bool hasViolation = false;
    warning_ = "";
    int rows = seatPlan.room.rows;
    int cols = seatPlan.room.columns;

    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows - 1; ++r) {
            if (r + 1 < static_cast<int>(seatPlan.grid.size()) && c < static_cast<int>(seatPlan.grid[r].size()) && c < static_cast<int>(seatPlan.grid[r + 1].size())) {
                const auto& s1 = seatPlan.grid[r][c];
                const auto& s2 = seatPlan.grid[r + 1][c];
                if (s1 != nullptr && s2 != nullptr) {
                    // We prefer same program vertically. If different, we generate a preference warning.
                    if (s1->program != s2->program) {
                        hasViolation = true;
                        warning_ += "Vertical Program Arrangement Preference: Different programs ('" + s1->program + 
                                    "' and '" + s2->program + "') are seated behind each other at Col " + std::to_string(c) + 
                                    ", Rows " + std::to_string(r) + " and " + std::to_string(r + 1) + ". ";
                    }
                }
            }
        }
    }
    // Returns false if preference isn't met, to flag warnings.
    return !hasViolation;
}

// ==========================================
// Rule 9: Different Program Same Department Rule
// ==========================================
bool DifferentProgramSameDeptRule::validate(const SeatPlan& seatPlan) {
    // This is an allowance rule. We check if there are seats where students are from the same department but different programs.
    // We log these as valid and verified relationships.
    warning_ = "";
    int rows = seatPlan.room.rows;
    int cols = seatPlan.room.columns;
    int verifiedPairs = 0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols - 1; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c + 1 < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& s1 = seatPlan.grid[r][c];
                const auto& s2 = seatPlan.grid[r][c + 1];
                if (s1 != nullptr && s2 != nullptr) {
                    if (s1->department == s2->department && s1->program != s2->program) {
                        verifiedPairs++;
                    }
                }
            }
        }
    }
    // Rule is inherently true as it represents a permitted state. 
    // We can document the verified permitted co-allocations in warning_ or log them.
    if (verifiedPairs > 0) {
        warning_ = "Info: Verified " + std::to_string(verifiedPairs) + 
                   " inter-program same-department horizontal pairings (e.g. CE next to AI under DoCSE).";
    }
    return true;
}

// ==========================================
// Rule 10: Same Subject / Exam Group Separation Rule
// ==========================================
bool SameSubjectSeparationRule::validate(const SeatPlan& seatPlan) {
    bool hasViolation = false;
    warning_ = "";
    int rows = seatPlan.room.rows;
    int cols = seatPlan.room.columns;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols - 1; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c + 1 < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& s1 = seatPlan.grid[r][c];
                const auto& s2 = seatPlan.grid[r][c + 1];
                if (s1 != nullptr && s2 != nullptr) {
                    if (s1->subject == s2->subject) {
                        hasViolation = true;
                        warning_ += "Same Subject Horizontal Violation: Students " + s1->name + " and " + s2->name + 
                                    " both writing same subject '" + s1->subject + "' are seated next to each other at Row " + 
                                    std::to_string(r) + ", Seats " + std::to_string(c) + " and " + std::to_string(c+1) + ". ";
                    }
                }
            }
        }
    }
    return !hasViolation;
}

// ==========================================
// Rule 11: Department/Program Distribution Rule
// ==========================================
bool DepartmentDistributionRule::validate(const SeatPlan& seatPlan) {
    int cols = seatPlan.room.columns;
    if (cols <= 1) {
        warning_ = "";
        return true;
    }

    int leftTotal = 0;
    int rightTotal = 0;
    std::unordered_map<std::string, int> leftProgramCounts;
    std::unordered_map<std::string, int> rightProgramCounts;

    int midCol = cols / 2;

    for (int r = 0; r < seatPlan.room.rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (r < static_cast<int>(seatPlan.grid.size()) && c < static_cast<int>(seatPlan.grid[r].size())) {
                const auto& seat = seatPlan.grid[r][c];
                if (seat != nullptr) {
                    if (c < midCol) {
                        leftTotal++;
                        leftProgramCounts[seat->program]++;
                    } else {
                        rightTotal++;
                        rightProgramCounts[seat->program]++;
                    }
                }
            }
        }
    }

    bool hasViolation = false;
    warning_ = "";

    // Check left side distribution dominance (> 60% of total)
    if (leftTotal > 0) {
        for (const auto& pair : leftProgramCounts) {
            const auto& prog = pair.first;
            const auto& count = pair.second;
            double percent = (static_cast<double>(count) / leftTotal) * 100.0;
            if (percent > 60.0) {
                hasViolation = true;
                warning_ += "Distribution Dominance: Program '" + prog + "' dominates the Left Side of the hall at " + 
                            std::to_string(static_cast<int>(percent)) + "% (" + std::to_string(count) + "/" + std::to_string(leftTotal) + "). ";
            }
        }
    }

    // Check right side distribution dominance (> 60% of total)
    if (rightTotal > 0) {
        for (const auto& pair : rightProgramCounts) {
            const auto& prog = pair.first;
            const auto& count = pair.second;
            double percent = (static_cast<double>(count) / rightTotal) * 100.0;
            if (percent > 60.0) {
                hasViolation = true;
                warning_ += "Distribution Dominance: Program '" + prog + "' dominates the Right Side of the hall at " + 
                            std::to_string(static_cast<int>(percent)) + "% (" + std::to_string(count) + "/" + std::to_string(rightTotal) + "). ";
            }
        }
    }

    return !hasViolation;
}

// ==========================================
// Rule 12: Invigilator Minimum Rule
// ==========================================
bool InvigilatorMinimumRule::validate(const SeatPlan& seatPlan) {
    if (seatPlan.room.numInvigilators < 2) {
        warning_ = "Invigilator Deficit: Room " + seatPlan.room.roomCode + " has only " + 
                   std::to_string(seatPlan.room.numInvigilators) + " invigilator(s). Minimum required is 2 (1 Chief, 1 Standard).";
        return false;
    }
    warning_ = "";
    return true;
}

// ==========================================
// Rule 13: Invigilator Capacity Ratio Rule
// ==========================================
bool InvigilatorRatioRule::validate(const SeatPlan& seatPlan) {
    int totalStudents = countTotalStudents(seatPlan);
    
    // 1 invigilator for every 10 students, but at least 2 total (from Rule 12 & Rule 13 select higher of the two)
    int requiredByRatio = (totalStudents + 9) / 10; // integer division ceil
    int finalRequired = std::max(2, requiredByRatio);

    if (seatPlan.room.numInvigilators < finalRequired) {
        warning_ = "Invigilator Ratio Violation: Room " + seatPlan.room.roomCode + " has " + 
                   std::to_string(seatPlan.room.numInvigilators) + " invigilators for " + std::to_string(totalStudents) + 
                   " students. Based on 1:10 ratio, required is " + std::to_string(finalRequired) + ".";
        return false;
    }
    warning_ = "";
    return true;
}

// ==========================================
// Rule 14: Manual Override and Rule Warning Rule
// ==========================================
bool ManualOverrideRule::validate(const SeatPlan& seatPlan) {
    // This checks if any seating assignment deviates from basic guidelines but remains manually approved.
    // If the room capacity has been manually adjusted or spacing rules have warnings, we list them.
    // Since this represents override logging, it checks if any standard overrideable warnings are present.
    // We can evaluate if there is any same-program horizontal or same-subject horizontal violations,
    // and label them as "Manual Override Warning".
    int warningsCount = 0;
    warning_ = "";

    // Example checks for manual overrides:
    SameProgramHorizontalRule r5;
    if (!r5.validate(seatPlan)) {
        warningsCount++;
        warning_ += "[Manual Override Active] Same Program next to each other. ";
    }

    SameSubjectSeparationRule r10;
    if (!r10.validate(seatPlan)) {
        warningsCount++;
        warning_ += "[Manual Override Active] Same Subject next to each other. ";
    }

    if (warningsCount > 0) {
        warning_ = "Structural Warning: Seating plan has " + std::to_string(warningsCount) + 
                   " manual override(s) active: " + warning_;
        return false; // Returns false to present warnings, but can be bypassed.
    }

    return true;
}
