#include "Room.h"
#include <stdexcept>
#include <sstream>

namespace nasala {




using namespace std;

//  Construction 
Room::Room(string id, string name, int rows, int cols,
           bool isolated, bool accessible)
    : roomID(move(id)), roomName(move(name)),
      rows(rows), columns(cols),
      isolated(isolated), accessible(accessible),
      assignedStudents(0)
{
    if (roomID.empty())
        throw invalid_argument("[Room] Room ID cannot be empty.");
    if (roomName.empty())
        throw invalid_argument("[Room] Room name cannot be empty.");
    if (rows <= 0 || cols <= 0)
        throw invalid_argument("[Room] Rows and columns must be positive integers.");

    capacity = rows * cols;
}

//  Identity getters 
const string& Room::getRoomID()   const { return roomID;   }
const string& Room::getRoomName() const { return roomName; }

//  Physical attribute getters 
int Room::getCapacity() const { return capacity; }
int Room::getRows()     const { return rows;     }
int Room::getColumns()  const { return columns;  }

//  Flag getters 
bool Room::isIsolated()   const { return isolated;   }
bool Room::isAccessible() const { return accessible; }

//  Occupancy getters 
int Room::getAssigned()  const { return assignedStudents;              }
int Room::getAvailable() const { return capacity - assignedStudents;   }

double Room::getOccupancyPercent() const {
    if (capacity == 0) return 0.0;
    return static_cast<double>(assignedStudents) * 100.0 / capacity;
}

//  Status 
RoomStatus Room::getStatus() const {
    if (assignedStudents >= capacity)      return RoomStatus::FULL;
    if (getOccupancyPercent() >= 80.0)     return RoomStatus::ALMOST_FULL;
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
    if (isolated)   badge += "[ISOLATED] ";
    if (accessible) badge += "[ACCESSIBLE]";
    if (badge.empty()) badge = "[STANDARD]";
    return badge;
}

//  Setters 
void Room::setRoomName(const string& name) {
    if (name.empty())
        throw invalid_argument("[Room] Room name cannot be empty.");
    roomName = name;
}

void Room::setIsolated  (bool val) { isolated   = val; }
void Room::setAccessible(bool val) { accessible = val; }

void Room::setDimensions(int r, int c) {
    if (r <= 0 || c <= 0)
        throw invalid_argument("[Room] Rows and columns must be positive integers.");
    int newCapacity = r * c;
    if (assignedStudents > newCapacity)
        throw invalid_argument(
            "[Room] Cannot shrink room: new capacity (" + to_string(newCapacity) +
            ") is less than assigned students (" + to_string(assignedStudents) + ").");
    rows     = r;
    columns  = c;
    capacity = newCapacity;
}

//  Seat management 
bool Room::hasAvailableSeat() const { return assignedStudents < capacity; }

bool Room::assignSeat() {
    if (!hasAvailableSeat()) return false;
    ++assignedStudents;
    return true;
}

void Room::removeSeat() {
    if (assignedStudents > 0) --assignedStudents;
}

void Room::resetAssignments() {
    assignedStudents = 0;
}

//  SQLite serialisation 
// Produces values string for:
//   INSERT INTO rooms(room_id, room_name, capacity, rows, columns, is_isolated, is_accessible)
string Room::toSQLValues() const {
    ostringstream oss;
    oss << "'"  << roomID   << "', "
        << "'"  << roomName << "', "
        << capacity << ", "
        << rows     << ", "
        << columns  << ", "
        << (isolated   ? 1 : 0) << ", "
        << (accessible ? 1 : 0);
    return oss.str();
}

//  Equality 
bool Room::operator==(const Room& other) const {
    return roomID == other.roomID;
}







} // namespace nasala
