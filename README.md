# KU Examination Seating System
### Kathmandu University — Office of the Controller of Examinations

A professional, production-grade Qt application for managing examination hall seating arrangements.

---

## Features

| Module | Capabilities |
|--------|-------------|
| **Dashboard** | Live stat cards, per-block occupancy bars, utilization overview |
| **Student Registry** | Add/Edit/Remove students, CSV import, search + filter by dept/semester |
| **Hall Seating** | Interactive 9-block hall map (A–I), click sub-blocks to assign seats |
| **Analytics** | Heatmap, block utilization bars, department breakdown table |
| **Export** | CSV student list, HTML printable seating chart, HTML block report |

### Hall Layout
- **9 Major Blocks** (A–I): arranged in 3 rows × 3 columns
- **3 Sub-blocks** per major block (e.g. A1, A2, A3)
- **6 Benches** per sub-block, **2 seats** per bench (Left / Right)
- **Total capacity: 324 seats** (9 × 3 × 6 × 2)

### Seat Status Colors
| Color | Meaning |
|-------|---------|
| 🟢 Green | Occupied |
| 🔵 Blue | Assigned |
| 🟡 Yellow | Reserved |
| 🔴 Red | Conflict |
| ⚪ Gray | Empty |

---

## Build Instructions

### Prerequisites
- Qt 5.15+ or Qt 6.x
- Qt Creator 8+ (recommended) or CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 10+, Clang 12+)

### Option A — Qt Creator (Recommended)
1. Open **Qt Creator**
2. File → Open File or Project
3. Select `KUExamSeating.pro`
4. Configure kit (Qt 5.15 or Qt 6)
5. Click **Build** (Ctrl+B) → **Run** (Ctrl+R)

### Option B — CMake (Qt Creator or command line)
```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64
cmake --build . --parallel
./KUExamSeating
```

### Option C — qmake command line
```bash
qmake KUExamSeating.pro
make -j4
./KUExamSeating
```

---

## Project Structure
```
KUExamSeating/
├── KUExamSeating.pro       # qmake project file
├── CMakeLists.txt          # CMake alternative
├── resources.qrc           # Qt resources
└── src/
    ├── main.cpp            # Entry point, app-wide stylesheet
    ├── mainwindow.h/.cpp   # Main window, sidebar navigation, menus
    ├── studentmodel.h/.cpp # Data model: students, seats, hall layout
    ├── hallmap.h/.cpp      # Interactive hall map widget (sub-block tiles)
    ├── subblockdialog.h/.cpp # Seat detail dialog (assign/unassign/lock)
    ├── dashboardpage.h/.cpp  # Dashboard with stat cards
    ├── studentspage.h/.cpp   # Student CRUD + table
    ├── seatingpage.h/.cpp    # Seating page with hall map + search
    ├── analyticspage.h/.cpp  # Heatmap + dept table
    └── exportmanager.h/.cpp  # CSV + HTML export
```

---

## CSV Import Format
Students can be bulk-imported via CSV with the following column order:
```
Name, Roll Number, Department, Semester, Program, Subject
```
Example:
```csv
Aarav Sharma,KU20001,Computer Science,5,BE,Data Structures
Binod Thapa,KU20002,Civil Engineering,3,BE,Surveying
```

---

## Color Theme
KU brand colors used throughout:
- **Primary Navy** `#003366` — main brand color
- **Accent Gold** `#c9a84c` — KU emblem gold
- **Accent Crimson** `#8b0000` — danger/warning
- **Page BG** `#f5f7fa` — light gray background

---

*KU ExamSeat v1.0 — Kathmandu University Office of Examinations*
