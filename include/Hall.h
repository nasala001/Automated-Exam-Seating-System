#ifndef HALL_H
#define HALL_H

#include <string>
#include <vector>
#include "Room.h"

class Hall {
private:
    std::string hallID;
    std::string hallName;
    std::vector<Room> rooms;

public:
    Hall(std::string id, std::string name);

    // Getters
    std::string hallGetID()        const;
    std::string getHallName()      const;
    int         getRoomCount()     const;
    int         getTotalCapacity() const;
    int         getTotalAssigned() const;
    int         getTotalAvailable() const;

    // Setter
    void setHallName(const std::string& name);

    // Room CRUD
    void  addRoom(const Room& room);
    bool  removeRoom(const std::string& roomID);
    Room* findRoom(const std::string& roomID);
    Room* findRoomByName(const std::string& name);
    bool  editRoom(const std::string& roomID,
                   const std::string& newName,
                   int rows, int cols);

    const std::vector<Room>& getRooms() const;
    std::vector<Room>&       getRooms();
};

#endif