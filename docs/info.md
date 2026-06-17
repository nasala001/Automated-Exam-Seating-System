# Automated Exam Seating Management System: Complete System Overview

The Automated Exam Seating Management System is designed to allocate students to physical venues, manage room capacities, handle special examination cases, and generate a final exam seating plan.

## 1. Core Allocation & Seating Rules

The system governs student placement using a strict set of rules to ensure fairness and efficiency:

* **Capacity & Assignment:** The system enforces a strict one-student-to-one-seat policy and ensures room capacities are never exceeded. If examinees exceed available seats, a capacity warning is triggered before final allocation.
* **Academic Separation:** To maintain exam integrity, the system prevents students from the same program, same semester/class, or same exam subject from sitting side-by-side horizontally. It also separates consecutive roll numbers horizontally.
* **Vertical Preferences:** While horizontal separation is enforced, students from the same program are preferably arranged vertically within the same column. The system also distributes programs across the hall so one program does not dominate a specific area.
* **Special Accommodations:** The system automatically adjusts for special cases, such as isolating students with contagious diseases  or modifying room capacities for students requiring a writing assistant due to physical disabilities.
* **Invigilation Constraints:** The system assigns a minimum of two invigilators (one Chief, one standard) to every room  and enforces a ratio of at least 1 invigilator per 10 examinees.
* **Manual Overrides:** Administrators can manually edit the seating arrangement, but the system will display a warning if the changes violate any of the established rules.

## 2. Seat Code Logic

Once seating is generated, the system creates a unique seat code for every student to help invigilators and examinees quickly identify the exact physical location.

* **Component Structure:** The code is built using the Hall/Block Code, the Group/Room Code, and the specific Seat Number.
* **Normal Room Format:** Formatted as `BLOCK-ROOM-SEATNO`.
  * *Example:* Block 9, Room 404, Seat 28 generates the code `B9-404-28`.
* **Large Hall Format:** Formatted as `HALLCODE-GROUP-SEATNO`.
  * *Example:* Multipurpose Hall, Group D, Seat 21 generates the code `MPH-D-21`.
* **Design Principle:** Program or department details (like CE or AI) are excluded from the seat code itself to focus purely on location, though they remain on the final report.

## 3. System Outputs

The final output of the system is a detailed Seating Plan. The system populates the following fields to ensure all necessary verification data is present:

* **Student Name:** Identifies the examinee.
* **Exam Roll No. & Registration No.:** Used for official identity verification and matching.
* **Program:** Displays the student's academic track (e.g., CE, AI, BIT) for rule checking.
* **Block / Hall / Room & Group:** Defines the physical venue and specific hall section if applicable.
* **Seat Number & Seat Code:** Displays the finalized seat number and the concatenated unique system code (e.g., `MPH-D-21`).
