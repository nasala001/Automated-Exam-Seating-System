#pragma once

#include <vector>
#include "../../shresh_validation/DataStructures.h"
#include "../../shresh_validation/RuleValidator.h"

namespace shresh {

struct AllocationResult {
    SeatPlan seatPlan;
    ValidationReport validationReport;
    bool success = false;
};

class SeatingEngine {
public:
    // Core engine function: Takes a list of students and a room, allocates seats deterministically,
    // and returns the SeatPlan along with the Shresh's validation report.
    static AllocationResult allocateSeats(std::vector<Student>& students, const Room& room);
};

} // namespace shresh

