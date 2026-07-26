#ifndef HALL_H

#define HALL_H





#include <string>
#include <vector>
#include "Room.h"

namespace nasala {


// ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
//  Hall  ÔÇô a physical exam venue containing Rooms
//
//  Responsibilities (your module only):
//    ÔÇó Own and manage a collection of Rooms
//    ÔÇó Provide aggregated capacity/availability data
//    ÔÇó Serialize itself for SQLite persistence
// ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
class Hall {
public:
    // ÔöÇÔöÇ Construction ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    Hall(std::string id, std::string name);

    // ÔöÇÔöÇ Identity getters ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    const std::string& getHallID()   const;   // FIXED: was hallGetID()
    const std::string& getHallName() const;

    // ÔöÇÔöÇ Setter ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    void setHallName(const std::string& name);

    // ÔöÇÔöÇ Aggregated stats ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    int getRoomCount()      const;
    int getTotalCapacity()  const;
    int getTotalAssigned()  const;
    int getTotalAvailable() const;
    double getOccupancyPercent() const;

    // ÔöÇÔöÇ Room CRUD ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    void  addRoom   (const Room& room);
    bool  removeRoom(const std::string& roomID);
    bool  editRoom  (const std::string& roomID,
                     const std::string& newName,
                     int rows, int cols);

    // ÔöÇÔöÇ Room queries ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    Room*       findRoom      (const std::string& roomID);
    const Room* findRoom      (const std::string& roomID) const;  // const overload
    Room*       findRoomByName(const std::string& name);
    const Room* findRoomByName(const std::string& name)   const;  // const overload

    // Returns rooms that have at least one free seat
    std::vector<Room*> getAvailableRooms();

    // Returns accessible rooms that have at least one free seat
    std::vector<Room*> getAccessibleRooms();

    // Returns isolated rooms that have at least one free seat
    std::vector<Room*> getIsolatedRooms();

    // ÔöÇÔöÇ Collection access ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    const std::vector<Room>& getRooms() const;
    std::vector<Room>&       getRooms();

    // ÔöÇÔöÇ SQLite serialisation helpers ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    // Returns values string for:
    //   INSERT INTO halls(hall_id, hall_name)
    std::string toSQLValues() const;

    // ÔöÇÔöÇ Equality ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
    bool operator==(const Hall& other) const;

private:
    std::string       hallID;
    std::string       hallName;
    std::vector<Room> rooms;
};








} // namespace nasala

#endif // HALL_H
