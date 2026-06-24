#include "Hall.h"
#include <stdexcept>
#include <sstream>

using namespace std;

// ── Construction ──────────────────────────────────────────────────────────────
Hall::Hall(string id, string name)
    : hallID(move(id)), hallName(move(name))
{
    if (hallID.empty())
        throw invalid_argument("[Hall] Hall ID cannot be empty.");
    if (hallName.empty())
        throw invalid_argument("[Hall] Hall name cannot be empty.");
}

// ── Identity getters ──────────────────────────────────────────────────────────
const string& Hall::getHallID()   const { return hallID;   }
const string& Hall::getHallName() const { return hallName; }

// ── Setter ────────────────────────────────────────────────────────────────────
void Hall::setHallName(const string& name) {
    if (name.empty())
        throw invalid_argument("[Hall] Hall name cannot be empty.");
    hallName = name;
}

// ── Aggregated stats ──────────────────────────────────────────────────────────
int Hall::getRoomCount() const {
    return static_cast<int>(rooms.size());
}

int Hall::getTotalCapacity() const {
    int total = 0;
    for (const auto& r : rooms) total += r.getCapacity();
    return total;
}

int Hall::getTotalAssigned() const {
    int total = 0;
    for (const auto& r : rooms) total += r.getAssigned();
    return total;
}

int Hall::getTotalAvailable() const {
    return getTotalCapacity() - getTotalAssigned();
}

double Hall::getOccupancyPercent() const {
    int cap = getTotalCapacity();
    if (cap == 0) return 0.0;
    return static_cast<double>(getTotalAssigned()) * 100.0 / cap;
}

// ── Room CRUD ─────────────────────────────────────────────────────────────────
void Hall::addRoom(const Room& room) {
    for (const auto& r : rooms)
        if (r.getRoomID() == room.getRoomID())
            throw invalid_argument(
                "[Hall] Room ID '" + room.getRoomID() + "' already exists in hall '" + hallName + "'.");
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

bool Hall::editRoom(const string& roomID, const string& newName, int rows, int cols) {
    Room* r = findRoom(roomID);
    if (!r) return false;
    r->setRoomName(newName);
    r->setDimensions(rows, cols);
    return true;
}

// ── Room queries ──────────────────────────────────────────────────────────────
Room* Hall::findRoom(const string& roomID) {
    for (auto& r : rooms)
        if (r.getRoomID() == roomID) return &r;
    return nullptr;
}

const Room* Hall::findRoom(const string& roomID) const {
    for (const auto& r : rooms)
        if (r.getRoomID() == roomID) return &r;
    return nullptr;
}

Room* Hall::findRoomByName(const string& name) {
    for (auto& r : rooms)
        if (r.getRoomName() == name) return &r;
    return nullptr;
}

const Room* Hall::findRoomByName(const string& name) const {
    for (const auto& r : rooms)
        if (r.getRoomName() == name) return &r;
    return nullptr;
}

vector<Room*> Hall::getAvailableRooms() {
    vector<Room*> result;
    for (auto& r : rooms)
        if (r.hasAvailableSeat()) result.push_back(&r);
    return result;
}

vector<Room*> Hall::getAccessibleRooms() {
    vector<Room*> result;
    for (auto& r : rooms)
        if (r.isAccessible() && r.hasAvailableSeat()) result.push_back(&r);
    return result;
}

vector<Room*> Hall::getIsolatedRooms() {
    vector<Room*> result;
    for (auto& r : rooms)
        if (r.isIsolated() && r.hasAvailableSeat()) result.push_back(&r);
    return result;
}

// ── Collection access ─────────────────────────────────────────────────────────
const vector<Room>& Hall::getRooms() const { return rooms; }
vector<Room>&       Hall::getRooms()       { return rooms; }

// ── SQLite serialisation ──────────────────────────────────────────────────────
// Produces values string for:
//   INSERT INTO halls(hall_id, hall_name)
string Hall::toSQLValues() const {
    ostringstream oss;
    oss << "'" << hallID << "', '" << hallName << "'";
    return oss.str();
}

// ── Equality ──────────────────────────────────────────────────────────────────
bool Hall::operator==(const Hall& other) const {
    return hallID == other.hallID;
}
