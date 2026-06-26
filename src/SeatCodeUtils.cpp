#include "SeatCodeUtils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

// Helper: trim leading/trailing whitespace
static string trim(const string &s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  size_t end = s.find_last_not_of(" \t\r\n");
  if (start == string::npos)
    return "";
  return s.substr(start, end - start + 1);
}

// Helper: uppercase a single character
static char toUpper(char c) {
  return static_cast<char>(toupper(static_cast<unsigned char>(c)));
}

// Helper: case-insensitive prefix check
static bool startsWithIgnoreCase(const string &s, const string &prefix) {
  if (s.size() < prefix.size())
    return false;
  for (size_t i = 0; i < prefix.size(); ++i) {
    if (tolower(static_cast<unsigned char>(s[i])) !=
        tolower(static_cast<unsigned char>(prefix[i]))) {
      return false;
    }
  }
  return true;
}

string normalizeVenueName(const string &fullName) {
  string name = trim(fullName);
  if (name.empty())
    return "UNKNOWN";

  // Known-alias table: covers common venue names whose abbreviations
  // are institutional conventions that cannot be derived algorithmically.
  // Keys are stored lowercase for case-insensitive matching.
  static const pair<string, string> knownAliases[] = {
      {"multipurpose hall", "MPH"},
      // Add more entries here as needed, e.g.:
      // { "central examination hall", "CEH" },
  };
  string nameLower = name;
  for (char &c : nameLower)
    c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
  for (const auto &alias : knownAliases) {
    if (nameLower == alias.first)
      return alias.second;
  }

  // Rule 1: "Block X" -> "BX"
  // Matches "Block" (case-insensitive) followed by optional space(s) and a
  // label
  if (startsWithIgnoreCase(name, "block")) {
    string rest = trim(name.substr(5)); // everything after "Block"
    if (!rest.empty()) {
      string code = "B";
      // Append the label (e.g. "9", "9A", "10") uppercased
      for (char c : rest) {
        code += toUpper(c);
      }
      return code;
    }
  }

  // Rule 2: Extract meaningful letters from each word.
  //   - For a word starting with uppercase: grab the first letter PLUS any
  //     subsequent uppercase letters within the same word (CamelCase scan).
  //     "Multipurpose" -> M, P  |  "Hall" -> H  =>  "MPH"
  //     "Main"         -> M     |  "Examination" -> E  |  "Center" -> C  =>
  //     "MEC"
  //   - For a word starting with a digit: append the leading digits as-is.
  string initials;
  istringstream iss(name);
  string word;
  while (iss >> word) {
    if (word.empty())
      continue;

    if (isupper(static_cast<unsigned char>(word[0]))) {
      // Always include the first (uppercase) letter
      initials += word[0];
      // Scan the rest of the word for additional uppercase letters
      for (size_t i = 1; i < word.size(); ++i) {
        if (isupper(static_cast<unsigned char>(word[i]))) {
          initials += word[i];
        }
      }
    } else if (isdigit(static_cast<unsigned char>(word[0]))) {
      for (char c : word) {
        if (isdigit(static_cast<unsigned char>(c)))
          initials += c;
        else
          break;
      }
    }
  }

  if (!initials.empty())
    return initials;

  // Fallback: uppercase the whole string (strip spaces)
  string fallback;
  for (char c : name) {
    if (!isspace(static_cast<unsigned char>(c)))
      fallback += toUpper(c);
  }
  return fallback;
}
