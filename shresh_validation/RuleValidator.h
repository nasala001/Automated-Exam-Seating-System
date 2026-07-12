#ifndef RULE_VALIDATOR_H
#define RULE_VALIDATOR_H

#include "SeatingRule.h"
#include <vector>
#include <memory>
#include <string>

namespace shresh {

struct ValidationReport {
    bool isValid = true;                       // True if no hard rules failed
    std::vector<std::string> hardErrors;       // Failures of rules where overrides are NOT allowed
    std::vector<std::string> softWarnings;     // Failures of rules where overrides are allowed or preferences
};

class RuleValidator {
private:
    std::vector<std::unique_ptr<SeatingRule>> rules_;

public:
    // Constructor: automatically registers all 14 standard rules
    RuleValidator();

    // Allows dynamic addition of custom or replacement rules
    void addRule(std::unique_ptr<SeatingRule> rule);

    // Clears all currently registered rules
    void clearRules();

    // Runs all registered rules against the seat plan, sorting failures into hard errors and soft warnings
    ValidationReport validate(const SeatPlan& seatPlan) const;
};

} // namespace shresh


#endif // RULE_VALIDATOR_H
