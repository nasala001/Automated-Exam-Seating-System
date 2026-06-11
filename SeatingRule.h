#ifndef SEATING_RULE_H
#define SEATING_RULE_H

#include "DataStructures.h"
#include <string>
#include <string_view>

class SeatingRule {
public:
    virtual ~SeatingRule() = default;

    // Validates the seat plan. Returns true if the rule is fully satisfied.
    // If it returns false, getWarningMessage() will contain details of the violations.
    virtual bool validate(const SeatPlan& seatPlan) = 0;

    // Returns the warning/error message of the validation failure.
    virtual std::string getWarningMessage() const = 0;

    // Returns a unique identifier or descriptive name for the rule.
    virtual std::string_view getRuleName() const = 0;

    // Indicates whether this rule can be manually overridden.
    // Rules like "One Student - One Seat" might be hard limits, whereas seating preferences are overrides.
    virtual bool isOverrideAllowed() const { return true; }
};

#endif // SEATING_RULE_H
