#include "engine/SeatingEngine.h"
#include <algorithm>
#include <map>
#include <cmath>

namespace shresh {

AllocationResult SeatingEngine::allocateSeats(std::vector<Student>& students, const Room& room) {
    AllocationResult result;
    result.seatPlan.room = room;
    result.seatPlan.grid.resize(room.rows, std::vector<std::shared_ptr<Student>>(room.columns, nullptr));

    std::vector<std::vector<bool>> blocked(room.rows, std::vector<bool>(room.columns, false));

    // 1. Separate students into categories: impaired, contagious, regular
    std::vector<Student> impairedStudents;
    std::vector<Student> contagiousStudents;
    std::vector<Student> regularStudents;

    for (const auto& s : students) {
        if (s.isPhysicallyImpaired) {
            impairedStudents.push_back(s);
        } else if (s.hasContagiousDisease) {
            contagiousStudents.push_back(s);
        } else {
            regularStudents.push_back(s);
        }
    }

    // 2. Allocate physically impaired students in Row 0
    int impairedCol = 0;
    for (const auto& s : impairedStudents) {
        while (impairedCol < room.columns && (result.seatPlan.grid[0][impairedCol] || blocked[0][impairedCol])) {
            impairedCol++;
        }
        if (impairedCol < room.columns) {
            result.seatPlan.grid[0][impairedCol] = std::make_shared<Student>(s);
        } else {
            // Fallback: place in row 0 anyway
        }
    }

    // 3. Allocate contagious disease students with 8-way isolation
    // We try to place them starting from the back row to minimize impact
    for (const auto& s : contagiousStudents) {
        bool placed = false;
        for (int r = room.rows - 1; r >= 0; --r) {
            for (int c = 0; c < room.columns; ++c) {
                if (result.seatPlan.grid[r][c] || blocked[r][c]) {
                    continue;
                }
                // Check if all 8 neighbors are free and not blocked
                bool neighborConflict = false;
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if (dr == 0 && dc == 0) continue;
                        int nr = r + dr;
                        int nc = c + dc;
                        if (nr >= 0 && nr < room.rows && nc >= 0 && nc < room.columns) {
                            if (result.seatPlan.grid[nr][nc] || blocked[nr][nc]) {
                                neighborConflict = true;
                                break;
                            }
                        }
                    }
                    if (neighborConflict) break;
                }

                if (!neighborConflict) {
                    // Place contagious student
                    result.seatPlan.grid[r][c] = std::make_shared<Student>(s);
                    blocked[r][c] = true;
                    // Block all 8 neighbors
                    for (int dr = -1; dr <= 1; ++dr) {
                        for (int dc = -1; dc <= 1; ++dc) {
                            int nr = r + dr;
                            int nc = c + dc;
                            if (nr >= 0 && nr < room.rows && nc >= 0 && nc < room.columns) {
                                blocked[nr][nc] = true;
                            }
                        }
                    }
                    placed = true;
                    break;
                }
            }
            if (placed) break;
        }
    }

    // 4. Separate regular students by program and section to interleave
    std::map<std::string, std::vector<Student>> programGroups;
    for (const auto& s : regularStudents) {
        std::string key = s.program;
        if (!s.section.empty()) {
            key += "_" + s.section;
        }
        programGroups[key].push_back(s);
    }
    
    std::vector<std::vector<Student>> buckets;
    for (auto& pair : programGroups) {
        buckets.push_back(pair.second);
    }
    
    // Sort buckets by size descending
    std::sort(buckets.begin(), buckets.end(), [](const auto& a, const auto& b) {
        return a.size() > b.size();
    });
    
    std::vector<int> indices(buckets.size(), 0);
    
    // Check if we have any students at all
    bool hasRegularStudents = false;
    for (const auto& b : buckets) {
        if (!b.empty()) {
            hasRegularStudents = true;
            break;
        }
    }

    if (hasRegularStudents) {
        int colBucket = -1;
        
        for (int c = 0; c < room.columns; ++c) {
            // Find a starting bucket for this column
            colBucket = -1;
            size_t maxLeft = 0;
            for (int i = 0; i < buckets.size(); ++i) {
                if (indices[i] < buckets[i].size()) {
                    bool matchesLeft = false;
                    if (c > 0) {
                        for (int r = 0; r < room.rows; ++r) {
                            if (result.seatPlan.grid[r][c-1]) {
                                if (result.seatPlan.grid[r][c-1]->section == buckets[i][indices[i]].section &&
                                    result.seatPlan.grid[r][c-1]->program == buckets[i][indices[i]].program) {
                                    matchesLeft = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!matchesLeft && (buckets[i].size() - indices[i] > maxLeft)) {
                        maxLeft = buckets[i].size() - indices[i];
                        colBucket = i;
                    }
                }
            }
            
            if (colBucket == -1) {
                maxLeft = 0;
                for (int i = 0; i < buckets.size(); ++i) {
                    if (indices[i] < buckets[i].size()) {
                        if (buckets[i].size() - indices[i] > maxLeft) {
                            maxLeft = buckets[i].size() - indices[i];
                            colBucket = i;
                        }
                    }
                }
            }
            
            if (colBucket == -1) {
                break; // Out of regular students
            }

            for (int r = 0; r < room.rows; ++r) {
                if (result.seatPlan.grid[r][c] || blocked[r][c]) {
                    continue; // Skip already assigned or blocked seats
                }

                // Check left neighbor conflict
                bool matchesLeft = false;
                if (indices[colBucket] < buckets[colBucket].size()) {
                    if (c > 0 && result.seatPlan.grid[r][c-1]) {
                        if (result.seatPlan.grid[r][c-1]->section == buckets[colBucket][indices[colBucket]].section &&
                            result.seatPlan.grid[r][c-1]->program == buckets[colBucket][indices[colBucket]].program) {
                            matchesLeft = true;
                        }
                    }
                } else {
                    matchesLeft = true;
                }

                if (matchesLeft || indices[colBucket] >= buckets[colBucket].size()) {
                    int bestBucket = -1;
                    size_t maxL = 0;
                    for (int i = 0; i < buckets.size(); ++i) {
                        if (indices[i] < buckets[i].size()) {
                            bool mLeft = false;
                            if (c > 0 && result.seatPlan.grid[r][c-1]) {
                                 if (result.seatPlan.grid[r][c-1]->section == buckets[i][indices[i]].section &&
                                     result.seatPlan.grid[r][c-1]->program == buckets[i][indices[i]].program) {
                                     mLeft = true;
                                 }
                            }
                            if (!mLeft && (buckets[i].size() - indices[i] > maxL)) {
                                maxL = buckets[i].size() - indices[i];
                                bestBucket = i;
                            }
                        }
                    }

                    if (bestBucket == -1) {
                        for (int i = 0; i < buckets.size(); ++i) {
                            if (indices[i] < buckets[i].size()) {
                                if (buckets[i].size() - indices[i] > maxL) {
                                    maxL = buckets[i].size() - indices[i];
                                    bestBucket = i;
                                }
                            }
                        }
                    }

                    if (bestBucket != -1) {
                        colBucket = bestBucket;
                    } else {
                        break; // Out of regular students for this column
                    }
                }

                result.seatPlan.grid[r][c] = std::make_shared<Student>(buckets[colBucket][indices[colBucket]++]);
            }
            
            // Check if we are completely out of regular students
            bool allRegularPlaced = true;
            for (int i = 0; i < buckets.size(); ++i) {
                if (indices[i] < buckets[i].size()) {
                    allRegularPlaced = false;
                    break;
                }
            }
            if (allRegularPlaced) {
                break;
            }
        }
    }
    
    // Apply RuleValidator
    RuleValidator validator;
    result.validationReport = validator.validate(result.seatPlan);
    result.success = result.validationReport.isValid;
    
    return result;
}

} // namespace shresh
