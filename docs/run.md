# How to Build & Run – Automated Exam Seating System

## Prerequisites

| Tool | Minimum Version | Notes |
|------|----------------|-------|
| [CMake](https://cmake.org/download/) | 3.16 | Add to PATH during install |
| C++ Compiler | C++17 support | See options below |

### Compiler Options (pick one)

| Compiler | Installation |
|----------|-------------|
| **MSVC** (Windows) | Install [Visual Studio](https://visualstudio.microsoft.com/) with the *Desktop development with C++* workload |
| **GCC / MinGW** (Windows) | Install [MSYS2](https://www.msys2.org/) and run `pacman -S mingw-w64-ucrt-x86_64-gcc cmake` |
| **GCC** (Linux / WSL) | `sudo apt install build-essential cmake` |

---

## Build Steps

### 1. Clone the repository
```bash
git clone https://github.com/nasala001/Automated-Exam-Seating-System.git
cd Automated-Exam-Seating-System
```

### 2. Create a build directory and configure
```bash
cmake -B build -S .
```

> **Using MinGW on Windows?** Add the generator flag:
> ```bash
> cmake -B build -S . -G "MinGW Makefiles"
> ```

### 3. Compile
```bash
cmake --build build
```

The executable will be placed in:
```
build/SeatCodeDemo          # Linux / macOS / MinGW
build/Debug/SeatCodeDemo.exe  # MSVC (Debug mode)
```

---

## Running the Demo

### Windows (MSVC)
```powershell
.\build\Debug\SeatCodeDemo.exe
```

### Windows (MinGW) / Linux
```bash
./build/SeatCodeDemo
```

---

## Example Session

```
==============================================
   Automated Exam Seating - Seat Code Demo
==============================================

Select venue type:
  [1] Exam Room  (e.g. Block 8, Room 304)
  [2] Large Hall (e.g. Multipurpose Hall, Group D)
  [0] Exit
Choice: 1

Enter block/building name (e.g. "Block 9"): Block 8
Enter room number (e.g. "304"): 304
Enter number of rows    : 2
Enter number of columns : 3

  Venue  : Block 8  [B8]
  Layout : 2 rows x 3 columns  (6 seats total)

         Col 1     Col 2     Col 3
      ----------------------------
R 1 |  B8-304-1  B8-304-2  B8-304-3
R 2 |  B8-304-4  B8-304-5  B8-304-6
```

```
Choice: 2

Enter hall name (e.g. "Multipurpose Hall"): Multipurpose Hall
Enter group/section label (e.g. "D"): D
Enter number of rows    : 2
Enter number of columns : 3

  Venue  : Multipurpose Hall  [MPH]
  Layout : 2 rows x 3 columns  (6 seats total)

       Col 1   Col 2   Col 3
      ----------------------
R 1 |  MPH-D-1  MPH-D-2  MPH-D-3
R 2 |  MPH-D-4  MPH-D-5  MPH-D-6
```

---

## Name Normalization Rules

| Input Name | Generated Code |
|-----------|---------------|
| `Block 9` | `B9` |
| `Block 10` | `B10` |
| `Multipurpose Hall` | `MPH` |
| `Main Examination Center` | `MEC` |
| `Science Block A` | `SBA` |

---

## Project Structure

```
Automated-Exam-Seating-System/
├── CMakeLists.txt
├── include/
│   ├── SeatCodeUtils.h    # normalizeVenueName() declaration
│   └── Venue.h            # Seat, Venue, ExamRoom, LargeHall declarations
├── src/
│   ├── main.cpp           # Interactive CLI demo
│   ├── SeatCodeUtils.cpp  # Dynamic name normalization logic
│   └── Venue.cpp          # Grid generation & seat code assignment
└── docs/
    ├── info.md            # System overview
    ├── Rules.md           # Seating rules
    └── run.md             # This file
```
