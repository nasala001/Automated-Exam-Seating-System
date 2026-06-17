#pragma once

#include <string>

// Normalizes a full venue name into a short code.
// Rules:
//   - "Block X"  (e.g. "Block 9")         -> "BX"  (e.g. "B9")
//   - Multi-word proper noun               -> initials of each capitalized word
//                                            (e.g. "Multipurpose Hall" -> "MPH",
//                                                   "Main Examination Center" -> "MEC")
//   - Fallback: returns the string uppercased as-is.
std::string normalizeVenueName(const std::string& fullName);
