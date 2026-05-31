#include "Hall.h"
#include <stdexcept>
#include <iostream>

using namespace std;

Hall::Hall(string id, string name) : hallID(id), hallName(name) {
    if (id.empty())   throw invalid_argument("[ERROR] Hall ID cannot be empty.");
    if (name.empty()) throw invalid_argument("[ERROR] Hall name cannot be empty.");
}

string Hall::hallGetID()    const { return hallID; }
string Hall::getHallName()  const { return hallName; }
int    Hall::getRoomCount() const { return (int)rooms.size(); }

int Hall::getTotalCapacity() const {
    int t = 0; for (const auto& r : rooms) t += r.getCapacity(); return t;
}
int Hall::getTotalAssigned() const {
    int t = 0; for (const auto& r : rooms) t += r.getAssigned(); return t;
}
int Hall::getTotalAvailable() const {
    return getTotalCapacity() - getTotalAssigned();
}

void Hall::setHallName(const string& name) {
    if (name.empty()) throw invalid_argument("[ERROR] Hall name cannot be empty.");
    hallName = name;
}

void Hall::addRoom(const Room& room) {
    for (const auto& r : rooms)
        if (r.getRoomID() == room.getRoomID())
            throw invalid_argument("[ERROR] Room ID '" + room.getRoomID() + "' already exists.");
    rooms.push_back(room);
}

bool Hall::removeRoom(const string& roomID) {
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        if (it->getRoomID() == roomID) {
            rooms.erase(it);
            return true;
        }
    }
    return false;
}

Room* Hall::findRoom(const string& roomID) {
    for (auto& r : rooms)
        if (r.getRoomID() == roomID) return &r;
    return nullptr;
}

Room* Hall::findRoomByName(const string& name) {
    for (auto& r : rooms)
        if (r.getRoomName() == name) return &r;
    return nullptr;
}

bool Hall::editRoom(const string& roomID,
                    const string& newName,
                    int rows, int cols)
{
    Room* r = findRoom(roomID);
    if (!r) return false;
    r->setRoomName(newName);
    r->setDimensions(rows, cols);
    return true;
}

const vector<Room>& Hall::getRooms() const { return rooms; }
vector<Room>&       Hall::getRooms()       { return rooms; }