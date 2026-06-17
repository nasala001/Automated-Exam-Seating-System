# Automated Exam Seating System: Complete Application Guide

## 1. What This Application Does

The Automated Exam Seating System is designed to generate fair, valid, and fast exam seating plans for an institution.

Instead of manually placing students room by room, the system reads student and exam data, applies seating rules, assigns seats automatically, validates constraints, and exports final seating charts for administration and students.

## 2. Problem It Solves

Manual seating allocation usually causes:

- Slow planning for large student counts
- Human mistakes in seat placement
- Rule violations (same group clustered, capacity overflow, etc.)
- Last-minute correction stress

This system solves that by using deterministic allocation logic, automatic validation, and export-ready outputs.

## 3. Core Users

- Exam Admin: uploads data, runs allocation, exports reports
- Invigilator/Faculty: reads room-wise and seat-wise plans
- Student: checks assigned room/seat

## 4. End-to-End Functional Flow

1. Import data (students, exams, rooms, rules)
2. Validate input files and detect data errors early
3. Build in-memory structures for students, rooms, and constraints
4. Run seat allocation engine
5. Validate output plan against all constraints
6. Show result in UI (summary + room details)
7. Export reports to CSV/PDF for distribution

## 5. Logical Modules

### 5.1 Data Import Module

Responsibilities:

- Read CSV files
- Parse rows safely
- Report invalid records with row references

Input examples:

- student list
- course/exam registrations
- room metadata

### 5.2 Domain Model Module

Responsibilities:

- Represent core entities in code
- Keep clean and reusable in-memory data structures

Main entities:

- Student
- Exam
- Room
- Seat
- AllocationRule
- AllocationResult

### 5.3 Rule Engine Module

Responsibilities:

- Apply institution-specific constraints before and during allocation

Typical rules:

- Room capacity cannot be exceeded
- Optional spacing (alternate seats/rows)
- Separation constraints by section or subject
- Special needs seat requirements

### 5.4 Seat Planning Engine

Responsibilities:

- Generate room-wise and seat-wise assignment
- Optimize for validity first, then balance/quality

Output:

- Student -> seat mapping
- Room occupancy summary

### 5.5 Validation Module

Responsibilities:

- Verify allocation result is conflict-free
- Produce actionable error details if any rule is violated

### 5.6 UI Module

Responsibilities:

- Allow file import and run controls
- Show progress and final seating
- Expose export actions

### 5.7 Export Module

Responsibilities:

- Save final outputs as CSV
- Optionally generate PDF layouts

## 6. Layer-by-Layer Tech Stack

This section shows both:

- Current stack already visible in the repository direction
- Recommended stack to complete the project robustly

| Layer | Current Direction | Recommended Final Stack | Why This Stack |
|---|---|---|---|
| Programming Language | C++ | C++20 | Strong performance and control for allocation logic |
| UI/Presentation | Qt framework | Qt 6 (Widgets or QML) | Cross-platform desktop UI, stable C++ integration |
| Application Logic | STL-based logic | C++ classes + STL + modular services | Predictable, testable business logic |
| Rule Engine | Planned | Strategy-pattern rule set in C++ | Easy to add/disable rules without rewriting core engine |
| Data Import/Export | CSV | CSV import/export via Qt file APIs or standard file streams | Keeps spreadsheet-friendly exchange for admins |
| Persistence | SQLite (selected) | SQLite3 (amalgamation: sqlite3.c + sqlite3.h) with relational schema | Single-file embedded DB, strong integrity, no server setup |
| Build System | Not defined yet | CMake + Ninja (or Make) | Standard C++ build flow, IDE-friendly, CI-friendly |
| Version Control | Git + GitHub | GitHub flow with branch PRs | Team collaboration and review safety |
| CI/CD | GitHub Actions (basic workflow exists) | GitHub Actions for build + test + lint | Detect issues early on every PR |
| Documentation | README + CONTRIBUTING | Markdown docs + architecture and API notes | Faster onboarding and maintenance |
| Packaging | Not defined yet | CPack or platform packaging scripts | Distributable desktop app artifacts |

## 7. Recommended Project Structure (Detailed)

Current top-level structure is minimal and clean:

- `include/` for headers
- `src/` for implementation

Recommended expansion:

```text
include/
  models/
  engine/
  rules/
  io/
  ui/
src/
  models/
  engine/
  rules/
  io/
  ui/
tests/
  unit/
  integration/
data/
  samples/
docs/
  architecture/
```

## 8. Data Contracts and Storage Model

Use CSV for inbound and outbound data exchange, and persist validated data in SQLite.

CSV schemas:

- students.csv: student_id, name, section, exam_code
- rooms.csv: room_id, capacity, building, floor
- rules.csv: rule_key, rule_value

SQLite core tables (minimum):

- students(id, student_id, name, section)
- exams(id, exam_code, exam_name)
- rooms(id, room_id, capacity, building, floor)
- exam_registrations(id, student_id_fk, exam_id_fk)
- allocation_runs(id, run_ts, ruleset_version, status)
- seat_allocations(id, run_id_fk, student_id_fk, room_id_fk, seat_label)

Validation and integrity rules:

- Required columns must exist
- IDs must be unique where needed
- Capacity and numeric fields must be valid positive integers
- Foreign keys must be enforced in SQLite
- Allocation records must be unique per run and seat

## 9. Allocation Algorithm Approach

Recommended strategy:

1. Pre-sort students by exam and section
2. Pre-sort rooms by capacity and constraints
3. Assign iteratively with rule checks at each candidate seat
4. Backtrack or re-balance when constraints conflict
5. Run final global validation pass

Target behavior:

- Deterministic output for same input
- Fast completion for large batches
- Human-readable conflict reports

## 10. Quality Gates Per Level

- Input level: reject invalid files early
- Rule level: each rule covered by unit tests
- Engine level: integration tests with sample datasets
- UI level: manual acceptance checklist
- CI level: build + tests required before merge

## 11. Security and Reliability Considerations

- Never trust CSV input; always validate and sanitize
- Handle duplicate IDs and malformed rows safely
- Keep non-crashing behavior on bad input
- Generate logs for import, allocation, and export events
- Use SQLite transactions for each allocation run to avoid partial writes

## 12. Suggested Roadmap

1. Finalize SQLite schema and CSV column mapping
2. Implement CSV importer and validator into SQLite tables
3. Build seat allocation engine against SQLite-backed repositories
4. Add rule engine and validation layer with transaction-safe writes
5. Integrate Qt UI and CSV/PDF export options
6. Add test suite and CI quality gates

## 13. Current Repository Reality Check

At the time of writing, `src/` and `include/` are placeholders only. This document defines the complete intended architecture and stack so implementation can proceed in a structured way.

## 14. Quick Stack Summary

- Core: C++20
- UI: Qt 6
- Data: SQLite3 (amalgamation) for persistence, CSV for import/export
- Build: CMake
- Testing: GoogleTest + CTest
- Quality: clang-tidy, clang-format, cppcheck
- Collaboration: GitHub PR workflow
- Automation: GitHub Actions
