# Automated Exam Seating Management System: Complete System Overview

[cite_start]The Automated Exam Seating Management System is designed to allocate students to physical venues, manage room capacities, handle special examination cases, and generate a final exam seating plan[cite: 4].

## 1. Core Allocation & Seating Rules
The system governs student placement using a strict set of rules to ensure fairness and efficiency:

* [cite_start]**Capacity & Assignment:** The system enforces a strict one-student-to-one-seat policy and ensures room capacities are never exceeded[cite: 6, 8, 9]. [cite_start]If examinees exceed available seats, a capacity warning is triggered before final allocation[cite: 7].
* [cite_start]**Academic Separation:** To maintain exam integrity, the system prevents students from the same program, same semester/class, or same exam subject from sitting side-by-side horizontally[cite: 17, 20, 33]. [cite_start]It also separates consecutive roll numbers horizontally[cite: 23].
* [cite_start]**Vertical Preferences:** While horizontal separation is enforced, students from the same program are preferably arranged vertically within the same column[cite: 26]. [cite_start]The system also distributes programs across the hall so one program does not dominate a specific area[cite: 37]. 
* [cite_start]**Special Accommodations:** The system automatically adjusts for special cases, such as isolating students with contagious diseases [cite: 11] [cite_start]or modifying room capacities for students requiring a writing assistant due to physical disabilities[cite: 14, 15].
* [cite_start]**Invigilation Constraints:** The system assigns a minimum of two invigilators (one Chief, one standard) to every room [cite: 39] [cite_start]and enforces a ratio of at least 1 invigilator per 10 examinees[cite: 41]. 
* [cite_start]**Manual Overrides:** Administrators can manually edit the seating arrangement, but the system will display a warning if the changes violate any of the established rules[cite: 44, 45].

## 2. Seat Code Logic
[cite_start]Once seating is generated, the system creates a unique seat code for every student to help invigilators and examinees quickly identify the exact physical location[cite: 49, 52]. 

* [cite_start]**Component Structure:** The code is built using the Hall/Block Code, the Group/Room Code, and the specific Seat Number[cite: 58, 59]. 
* [cite_start]**Normal Room Format:** Formatted as `BLOCK-ROOM-SEATNO`[cite: 55]. 
    * [cite_start]*Example:* Block 9, Room 404, Seat 28 generates the code `B9-404-28`[cite: 68, 71, 72].
* [cite_start]**Large Hall Format:** Formatted as `HALLCODE-GROUP-SEATNO`[cite: 57]. 
    * [cite_start]*Example:* Multipurpose Hall, Group D, Seat 21 generates the code `MPH-D-21`[cite: 64, 65, 66].
* [cite_start]**Design Principle:** Program or department details (like CE or AI) are excluded from the seat code itself to focus purely on location, though they remain on the final report[cite: 61, 62].

## 3. System Outputs
The final output of the system is a detailed Seating Plan. [cite_start]The system populates the following fields to ensure all necessary verification data is present[cite: 73]:

* [cite_start]**Student Name:** Identifies the examinee[cite: 74].
* [cite_start]**Exam Roll No. & Registration No.:** Used for official identity verification and matching[cite: 74].
* [cite_start]**Program:** Displays the student's academic track (e.g., CE, AI, BIT) for rule checking[cite: 74].
* [cite_start]**Block / Hall / Room & Group:** Defines the physical venue and specific hall section if applicable[cite: 74].
* [cite_start]**Seat Number & Seat Code:** Displays the finalized seat number and the concatenated unique system code (e.g., `MPH-D-21`)[cite: 74].