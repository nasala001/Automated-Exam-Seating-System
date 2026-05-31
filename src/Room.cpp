#include "Room.h"
#include <stdexcept>

using namespace std;

Room::Room(string id, string name, int rows, int cols,
           bool isolated, bool accessible)
    : roomID(id), roomName(name), rows(rows), columns(cols),
      isIsolated(isolated), isAccessible(accessible), assignedStudents(0)
{
    if (id.empty())
        throw invalid_argument("[ERROR] Room ID cannot be empty.");
    if (name.empty())
        throw invalid_argument("[ERROR] Room name cannot be empty.");
    if (rows <= 0 || cols <= 0)
        throw invalid_argument("[ERROR] Rows and columns must be positive.");
    capacity = rows * cols;
}

string Room::getRoomID()       const { return roomID; }
string Room::getRoomName()     const { return roomName; }
int    Room::getCapacity()     const { return capacity; }
int    Room::getRows()         const { return rows; }
int    Room::getColumns()      const { return columns; }
bool   Room::getIsIsolated()   const { return isIsolated; }
bool   Room::getIsAccessible() const { return isAccessible; }
int    Room::getAssigned()     const { return assignedStudents; }
int    Room::getAvailable()    const { return capacity - assignedStudents; }

double Room::getOccupancyPercent() const {
    if (capacity == 0) return 0.0;
    return (double)assignedStudents * 100.0 / capacity;
}

RoomStatus Room::getStatus() const {
    if (assignedStudents >= capacity) return RoomStatus::FULL;
    if (getOccupancyPercent() >= 80.0) return RoomStatus::ALMOST_FULL;
    return RoomStatus::AVAILABLE;
}

string Room::getStatusString() const {
    switch (getStatus()) {
        case RoomStatus::FULL:        return "FULL";
        case RoomStatus::ALMOST_FULL: return "ALMOST FULL";
        default:                      return "AVAILABLE";
    }
}

string Room::getBadge() const {
    string badge;
    if (isIsolated)   badge += "[ISOLATED] ";
    if (isAccessible) badge += "[ACCESSIBLE]";
    return badge;
}

void Room::setRoomName(const string& name) {
    if (name.empty())
        throw invalid_argument("[ERROR] Room name cannot be empty.");
    roomName = name;
}
void Room::setIsolated(bool val)   { isIsolated = val; }
void Room::setAccessible(bool val) { isAccessible = val; }

// FIXED: prevents shrinking below assigned count
void Room::setDimensions(int r, int c) {
    if (r <= 0 || c <= 0)
        throw invalid_argument("[ERROR] Rows and columns must be positive.");
    int newCapacity = r * c;
    if (assignedStudents > newCapacity)
        throw invalid_argument("[ERROR] New capacity is less than assigned students.");
    rows = r; columns = c; capacity = newCapacity;
}

bool Room::hasAvailableSeat() const { return assignedStudents < capacity; }

bool Room::assignSeat() {
    if (!hasAvailableSeat()) return false;
    assignedStudents++;
    return true;
}
void Room::removeSeat()       { if (assignedStudents > 0) assignedStudents--; }
void Room::resetAssignments() { assignedStudents = 0; }