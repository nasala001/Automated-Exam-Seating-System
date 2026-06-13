#ifndef HALLMANAGER_H
#define HALLMANAGER_H

#include <vector>
#include <string>
#include "Hall.h"

// ─────────────────────────────────────────────
//  HallManager  – owns and manages all Halls
//
//  Responsibilities (your module only):
//    • Top-level CRUD for Halls
//    • System-wide capacity and availability queries
//    • Entry point for the allocation engine to
//      ask "give me rooms that satisfy X"
//    • Will connect to SQLite repository layer
// ─────────────────────────────────────────────
class HallManager {
public:
    // ── Hall CRUD ─────────────────────────────
    void addHall(const Hall& hall);
    bool removeHall(const std::string& hallID);
    bool editHallName(const std::string& hallID, const std::string& newName);

    // ── Hall queries ──────────────────────────
    Hall* findHall(const std::string& hallID);
    const Hall* findHall(const std::string& hallID) const;  // const overload
    Hall* findHallByName(const std::string& name);
    const Hall* findHallByName(const std::string& name)   const;  // const overload

    // ── System-wide capacity queries ──────────
    int getTotalSystemCapacity()  const;
    int getTotalSystemAssigned()  const;
    int getTotalSystemAvailable() const;

    // ── Capacity warning (used before allocation run) ─
    // Returns true if enough seats exist, false otherwise
    bool checkCapacity(int totalStudents) const;

    // ── Engine-facing queries ─────────────────
    // Returns all rooms across all halls with available seats
    std::vector<Room*> getAllAvailableRooms();

    // Returns all accessible rooms with available seats
    std::vector<Room*> getAllAccessibleRooms();

    // Returns all isolated rooms with available seats
    std::vector<Room*> getAllIsolatedRooms();

    // Returns rooms with capacity >= minCapacity and available seats
    std::vector<Room*> getRoomsWithMinCapacity(int minCapacity);

    // ── Collection access ─────────────────────
    const std::vector<Hall>& getHalls() const;
    std::vector<Hall>& getHalls();

    // ── Utility ───────────────────────────────
    bool isEmpty()   const;
    int  hallCount() const;

    // Resets ALL seat assignments across all halls and rooms
    void resetAllAssignments();

private:
    std::vector<Hall> halls;
};

#endif // HALLMANAGER_H
