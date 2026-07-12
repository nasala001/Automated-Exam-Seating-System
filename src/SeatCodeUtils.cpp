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

  // Normalize underscores to spaces (handles IDs like HALL_B, DRAWING_HALL)
  string normalized = name;
  for (char &c : normalized)
    if (c == '_') c = ' ';
  normalized = trim(normalized);

  // Known-alias table (case-insensitive full match after normalization)
  static const pair<string, string> knownAliases[] = {
      {"multipurpose hall",        "MPH"},
      {"multi purpose hall",       "MPH"},
      {"multi-purpose hall",       "MPH"},
      {"main hall",                "MH"},
      {"central examination hall", "CEH"},
      {"drawing hall",             "DH"},
      {"lecture hall",             "LH"},
      {"computer lab",             "CL"},
      {"seminar hall",             "SH"},
      {"ku central exam hall",     "KU"},
  };
  string normLower = normalized;
  for (char &c : normLower)
    c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
  for (const auto &alias : knownAliases) {
    if (normLower == alias.first)
      return alias.second;
  }

  // Rule 1: "Block X" -> "BX"
  if (startsWithIgnoreCase(normalized, "block")) {
    string rest = trim(normalized.substr(5));
    if (!rest.empty()) {
      string code = "B";
      for (char c : rest) if (!isspace((unsigned char)c)) code += toUpper(c);
      return code;
    }
  }

  // Rule 2: "Hall X" -> "HX"  (e.g. "Hall B" -> "HB")
  if (startsWithIgnoreCase(normalized, "hall")) {
    string rest = trim(normalized.substr(4));
    if (!rest.empty()) {
      string code = "H";
      for (char c : rest) if (!isspace((unsigned char)c)) code += toUpper(c);
      return code;
    }
  }

  // Rule 3: initials from each meaningful word, skipping noise words
  static const char* noiseWords[] = {"the","of","and","at","in","for","a","an",nullptr};
  auto isNoise = [](const string &w) -> bool {
    string wl = w;
    for (char &c : wl) c = (char)tolower((unsigned char)c);
    for (const char** n = noiseWords; *n; ++n)
      if (wl == *n) return true;
    return false;
  };

  string initials;
  istringstream iss(normalized);
  string word;
  while (iss >> word) {
    if (word.empty() || isNoise(word)) continue;
    if (isdigit((unsigned char)word[0])) {
      for (char c : word) { if (isdigit((unsigned char)c)) initials += c; else break; }
    } else {
      initials += toUpper(word[0]);
      for (size_t i = 1; i < word.size(); ++i)
        if (isupper((unsigned char)word[i])) initials += word[i];
    }
  }
  if (!initials.empty()) return initials;

  // Fallback: strip spaces, uppercase everything
  string fallback;
  for (char c : normalized)
    if (!isspace((unsigned char)c)) fallback += toUpper(c);
  return fallback;
}

