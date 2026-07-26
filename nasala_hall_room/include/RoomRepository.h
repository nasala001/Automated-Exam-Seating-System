#ifndef ROOM_REPOSITORY_H

#define ROOM_REPOSITORY_H



#include <string>
#include <vector>
#include "Room.h"
#include "sqlite3.h"

namespace nasala {


// 
//  RoomRepository
//  Handles all SQLite read/write operations for Room objects.
//  Does NOT own the db connection  it is passed in (shared with HallRepository).
// 
class RoomRepository {
public:
    explicit RoomRepository(sqlite3* db);

    // Insert a new room linked to a hall. Returns false on failure.
    bool save(const Room& room, const std::string& hallID);

    // Load every room belonging to a given hall.
    std::vector<Room> loadByHallID(const std::string& hallID);

    // Update an existing room's mutable fields (name, dimensions, flags).
    bool update(const Room& room);

    // Delete a room by its room_id.
    bool remove(const std::string& roomID);

    // Returns true if a room with this ID already exists.
    bool exists(const std::string& roomID);

private:
    sqlite3* db;  // not owned
};






} // namespace nasala

#endif // ROOM_REPOSITORY_H



