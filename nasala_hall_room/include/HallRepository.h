#ifndef HALL_REPOSITORY_H

#define HALL_REPOSITORY_H



#include <string>
#include <vector>
#include "Hall.h"
#include "RoomRepository.h"
#include "sqlite3.h"

namespace nasala {


// 
//  HallRepository
//  Handles all SQLite read/write operations for Hall objects.
//  Internally uses RoomRepository to persist each Hall's rooms.
// 
class HallRepository {
public:
    explicit HallRepository(sqlite3* db);

    // Opens (or creates) the database file and runs schema.sql.
    // Returns nullptr on failure.
    static sqlite3* openDatabase(const std::string& dbPath);

    // Saves a hall AND all of its rooms in a single transaction.
    bool save(const Hall& hall);

    // Loads every hall from the DB, each with its rooms populated.
    std::vector<Hall> loadAll();

    // Loads a single hall (with rooms) by its hall_id.
    bool loadByID(const std::string& hallID, Hall& outHall);

    bool update(const Hall& hall);
    bool remove(const std::string& hallID);
    bool exists(const std::string& hallID);

private:
    sqlite3*       db;    // not owned
    RoomRepository roomRepo;
};






} // namespace nasala

#endif // HALL_REPOSITORY_H



