#ifndef CONCRETE_RULES_H
#define CONCRETE_RULES_H

#include "SeatingRule.h"
#include <string>
#include <string_view>

// Rule 1: Room Capacity Rule
// Ensures that the total number of assigned students does not exceed the room capacity.
class RoomCapacityRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 1: Room Capacity Rule"; }
    bool isOverrideAllowed() const override { return false; } // Capacity is a hard physical limit
};

// Rule 2: One Student - One Seat Rule
// Ensures that every assigned student has a unique seat, meaning no duplicate student registrations/roll numbers.
class OneStudentOneSeatRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 2: One Student - One Seat Rule"; }
    bool isOverrideAllowed() const override { return false; } // Hard logical constraint
};

// Rule 3: Contagious Disease Isolation Rule
// Ensures that students with a contagious disease are isolated (all adjacent 8 seats must be empty).
class ContagiousDiseaseIsolationRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 3: Contagious Disease Isolation Rule"; }
    bool isOverrideAllowed() const override { return false; } // Critical health safety rule
};

// Rule 4: Physical Disability and Amanuensis Rule
// Ensures physically impaired students are placed in accessible seats (e.g., front row: row 0)
// and adjusts room capacity calculation dynamically to account for their assistants (amanuenses).
class PhysicalDisabilityRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 4: Physical Disability and Amanuensis Rule"; }
    bool isOverrideAllowed() const override { return true; }
};

// Rule 5: Same Program Horizontal Separation Rule
// Prevents students from the same program (e.g., CE next to CE) from being seated side-by-side horizontally.
class SameProgramHorizontalRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 5: Same Program Horizontal Separation Rule"; }
    bool isOverrideAllowed() const override { return true; }
};

// Rule 6: Same Class/Semester Separation Rule
// Prevents students from the same class/semester (determined by program and registration/batch year)
// from sitting side-by-side horizontally.
class SameSemesterSeparationRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 6: Same Class/Semester Separation Rule"; }
    bool isOverrideAllowed() const override { return true; }
};

// Rule 7: Consecutive Roll Number Horizontal Separation Rule
// Prevents students with consecutive roll numbers from sitting side-by-side horizontally.
class ConsecutiveRollSeparationRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 7: Consecutive Roll Number Horizontal Separation Rule"; }
    bool isOverrideAllowed() const override { return true; }
};

// Rule 8: Same Program Vertical Arrangement Preference
// Expresses a preference that students of the same program should sit behind each other (in the same column).
class SameProgramVerticalPreferenceRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 8: Same Program Vertical Arrangement Preference"; }
    bool isOverrideAllowed() const override { return true; }
};

// Rule 9: Different Program Same Department Rule
// Validates and verifies that different programs in the same department (e.g., AI and CE under DoCSE)
// are allowed side-by-side, ensuring they are not flagged as violations by department metrics.
class DifferentProgramSameDeptRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 9: Different Program Same Department Rule"; }
    bool isOverrideAllowed() const override { return true; }
};

// Rule 10: Same Subject / Exam Group Separation Rule
// Prevents side-by-side horizontal seating of students writing the same subject/exam paper.
class SameSubjectSeparationRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 10: Same Subject / Exam Group Separation Rule"; }
    bool isOverrideAllowed() const override { return true; }
};

// Rule 11: Department/Program Distribution Rule
// Checks that the seating is evenly distributed across the hall, so no program dominates more than 60% of any half of the room.
class DepartmentDistributionRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 11: Department/Program Distribution Rule"; }
    bool isOverrideAllowed() const override { return true; }
};

// Rule 12: Invigilator Minimum Rule
// Every hall must have at least 2 invigilators: 1 Chief and 1 Standard.
class InvigilatorMinimumRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 12: Invigilator Minimum Rule"; }
    bool isOverrideAllowed() const override { return false; } // Admin/policy hard limit
};

// Rule 13: Invigilator Capacity Ratio Rule
// Ensures a 1:10 ratio of invigilators to students. The active required number is the higher of Rule 12 and Rule 13.
class InvigilatorRatioRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 13: Invigilator Capacity Ratio Rule"; }
    bool isOverrideAllowed() const override { return false; } // Admin/policy hard limit
};

// Rule 14: Manual Override and Rule Warning Rule
// Inspects the SeatPlan to see if there are manual modifications that override standard rules,
// returning structural warnings for override actions.
class ManualOverrideRule : public SeatingRule {
private:
    std::string warning_;
public:
    bool validate(const SeatPlan& seatPlan) override;
    std::string getWarningMessage() const override { return warning_; }
    std::string_view getRuleName() const override { return "Rule 14: Manual Override and Rule Warning Rule"; }
    bool isOverrideAllowed() const override { return true; }
};

#endif // CONCRETE_RULES_H
