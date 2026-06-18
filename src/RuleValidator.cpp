#include "RuleValidator.h"
#include "ConcreteRules.h"
#include <utility>

RuleValidator::RuleValidator() {
    // Register all 14 predefined seating and management rules
    rules_.push_back(std::make_unique<RoomCapacityRule>());
    rules_.push_back(std::make_unique<OneStudentOneSeatRule>());
    rules_.push_back(std::make_unique<ContagiousDiseaseIsolationRule>());
    rules_.push_back(std::make_unique<PhysicalDisabilityRule>());
    rules_.push_back(std::make_unique<SameProgramHorizontalRule>());
    rules_.push_back(std::make_unique<SameSemesterSeparationRule>());
    rules_.push_back(std::make_unique<ConsecutiveRollSeparationRule>());
    rules_.push_back(std::make_unique<SameProgramVerticalPreferenceRule>());
    rules_.push_back(std::make_unique<DifferentProgramSameDeptRule>());
    rules_.push_back(std::make_unique<SameSubjectSeparationRule>());
    rules_.push_back(std::make_unique<DepartmentDistributionRule>());
    rules_.push_back(std::make_unique<InvigilatorMinimumRule>());
    rules_.push_back(std::make_unique<InvigilatorRatioRule>());
    rules_.push_back(std::make_unique<ManualOverrideRule>());
}

void RuleValidator::addRule(std::unique_ptr<SeatingRule> rule) {
    if (rule) {
        rules_.push_back(std::move(rule));
    }
}

void RuleValidator::clearRules() {
    rules_.clear();
}

ValidationReport RuleValidator::validate(const SeatPlan& seatPlan) const {
    ValidationReport report;
    report.isValid = true;

    for (const auto& rule : rules_) {
        // Evaluate rule
        if (!rule->validate(seatPlan)) {
            std::string violationMsg = std::string(rule->getRuleName()) + ": " + rule->getWarningMessage();
            if (rule->isOverrideAllowed()) {
                report.softWarnings.push_back(violationMsg);
            } else {
                report.isValid = false; // Hard failure flags the seating plan as invalid
                report.hardErrors.push_back(violationMsg);
            }
        }
    }

    return report;
}
