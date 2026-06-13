#ifndef HALL_H
#define HALL_H

#include <string>
#include <vector>
#include "Room.h"

// ─────────────────────────────────────────────
//  Hall  – a physical exam venue containing Rooms
//
//  Responsibilities (your module only):
//    • Own and manage a collection of Rooms
//    • Provide aggregated capacity/availability data
//    • Serialize itself for SQLite persistence
// ─────────────────────────────────────────────
class Hall {
public:
    // ── Construction ─────────────────────────
    Hall(std::string id, std::string name);

    // ── Identity getters ─────────────────────
    const std::string& getHallID()   const;   // FIXED: was hallGetID()
    const std::string& getHallName() const;

    // ── Setter ───────────────────────────────
    void setHallName(const std::string& name);

    // ── Aggregated stats ──────────────────────
    int getRoomCount()      const;
    int getTotalCapacity()  const;
    int getTotalAssigned()  const;
    int getTotalAvailable() const;
    double getOccupancyPercent() const;

    // ── Room CRUD ─────────────────────────────
    void  addRoom(const Room& room);
    bool  removeRoom(const std::string& roomID);
    bool  editRoom(const std::string& roomID,
        const std::string& newName,
        int rows, int cols);

    // ── Room queries ──────────────────────────
    Room* findRoom(const std::string& roomID);
    const Room* findRoom(const std::string& roomID) const;  // const overload
    Room* findRoomByName(const std::string& name);
    const Room* findRoomByName(const std::string& name)   const;  // const overload

    // Returns rooms that have at least one free seat
    std::vector<Room*> getAvailableRooms();

    // Returns accessible rooms that have at least one free seat
    std::vector<Room*> getAccessibleRooms();

    // Returns isolated rooms that have at least one free seat
    std::vector<Room*> getIsolatedRooms();

    // ── Collection access ─────────────────────
    const std::vector<Room>& getRooms() const;
    std::vector<Room>& getRooms();

    // ── SQLite serialisation helpers ──────────
    // Returns values string for:
    //   INSERT INTO halls(hall_id, hall_name)
    std::string toSQLValues() const;

    // ── Equality ─────────────────────────────
    bool operator==(const Hall& other) const;

private:
    std::string       hallID;
    std::string       hallName;
    std::vector<Room> rooms;
};

#endif // HALL_H
