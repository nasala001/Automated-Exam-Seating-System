#include "engine/SeatingEngine.h"
#include <algorithm>
#include <map>

AllocationResult SeatingEngine::allocateSeats(std::vector<Student>& students, const Room& room) {
    AllocationResult result;
    result.seatPlan.room = room;
    result.seatPlan.grid.resize(room.rows, std::vector<std::shared_ptr<Student>>(room.columns, nullptr));
    
    // Separate by program to interleave and prevent same program adjacent
    std::map<std::string, std::vector<Student>> programGroups;
    for (const auto& s : students) {
        programGroups[s.program].push_back(s);
    }
    
    std::vector<std::vector<Student>> buckets;
    for (auto& pair : programGroups) {
        buckets.push_back(pair.second);
    }
    
    // Sort buckets by size descending to ensure largest groups are spread out well
    std::sort(buckets.begin(), buckets.end(), [](const auto& a, const auto& b) {
        return a.size() > b.size();
    });
    
    std::vector<int> indices(buckets.size(), 0);
    int currentBucket = 0;
    
    for (int r = 0; r < room.rows; ++r) {
        for (int c = 0; c < room.columns; ++c) {
            // Find the next available student, trying different buckets
            bool placed = false;
            for (int i = 0; i < buckets.size(); ++i) {
                int bIdx = (currentBucket + i) % buckets.size();
                if (indices[bIdx] < buckets[bIdx].size()) {
                    result.seatPlan.grid[r][c] = std::make_shared<Student>(buckets[bIdx][indices[bIdx]++]);
                    currentBucket = (bIdx + 1) % buckets.size();
                    placed = true;
                    break;
                }
            }
            if (!placed) {
                break; // No more students left to place
            }
        }
        // Offset starting program for next row to prevent vertical alignment if columns % buckets == 0
        if (buckets.size() > 0 && room.columns % buckets.size() == 0) {
            currentBucket = (currentBucket + 1) % buckets.size();
        }
    }
    
    // Apply Shresh's RuleValidator
    RuleValidator validator;
    result.validationReport = validator.validate(result.seatPlan);
    result.success = result.validationReport.isValid;
    
    return result;
}
