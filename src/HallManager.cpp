#include "HallManager.h"
#include <stdexcept>

using namespace std;

// ── Hall CRUD ─────────────────────────────────────────────────────────────────
void HallManager::addHall(const Hall& hall) {
    for (const auto& h : halls)
        if (h.getHallID() == hall.getHallID())
            throw invalid_argument(
                "[HallManager] Hall ID '" + hall.getHallID() + "' already exists.");
    halls.push_back(hall);
}

bool HallManager::removeHall(const string& hallID) {
    for (auto it = halls.begin(); it != halls.end(); ++it) {
        if (it->getHallID() == hallID) {
            halls.erase(it);
            return true;
        }
    }
    return false;
}

bool HallManager::editHallName(const string& hallID, const string& newName) {
    Hall* h = findHall(hallID);
    if (!h) return false;
    h->setHallName(newName);
    return true;
}

// ── Hall queries ──────────────────────────────────────────────────────────────
Hall* HallManager::findHall(const string& hallID) {
    for (auto& h : halls)
        if (h.getHallID() == hallID) return &h;
    return nullptr;
}

const Hall* HallManager::findHall(const string& hallID) const {
    for (const auto& h : halls)
        if (h.getHallID() == hallID) return &h;
    return nullptr;
}

Hall* HallManager::findHallByName(const string& name) {
    for (auto& h : halls)
        if (h.getHallName() == name) return &h;
    return nullptr;
}

const Hall* HallManager::findHallByName(const string& name) const {
    for (const auto& h : halls)
        if (h.getHallName() == name) return &h;
    return nullptr;
}

// ── System-wide capacity queries ──────────────────────────────────────────────
int HallManager::getTotalSystemCapacity() const {
    int total = 0;
    for (const auto& h : halls) total += h.getTotalCapacity();
    return total;
}

int HallManager::getTotalSystemAssigned() const {
    int total = 0;
    for (const auto& h : halls) total += h.getTotalAssigned();
    return total;
}

int HallManager::getTotalSystemAvailable() const {
    return getTotalSystemCapacity() - getTotalSystemAssigned();
}

bool HallManager::checkCapacity(int totalStudents) const {
    int seats = getTotalSystemCapacity();
    return totalStudents <= seats;
}

// ── Engine-facing queries ─────────────────────────────────────────────────────
vector<Room*> HallManager::getAllAvailableRooms() {
    vector<Room*> result;
    for (auto& h : halls) {
        auto available = h.getAvailableRooms();
        result.insert(result.end(), available.begin(), available.end());
    }
    return result;
}

vector<Room*> HallManager::getAllAccessibleRooms() {
    vector<Room*> result;
    for (auto& h : halls) {
        auto accessible = h.getAccessibleRooms();
        result.insert(result.end(), accessible.begin(), accessible.end());
    }
    return result;
}

vector<Room*> HallManager::getAllIsolatedRooms() {
    vector<Room*> result;
    for (auto& h : halls) {
        auto isolated = h.getIsolatedRooms();
        result.insert(result.end(), isolated.begin(), isolated.end());
    }
    return result;
}

vector<Room*> HallManager::getRoomsWithMinCapacity(int minCapacity) {
    vector<Room*> result;
    for (auto& h : halls)
        for (auto& r : h.getRooms())
            if (r.getCapacity() >= minCapacity && r.hasAvailableSeat())
                result.push_back(&r);
    return result;
}

// ── Collection access ─────────────────────────────────────────────────────────
const vector<Hall>& HallManager::getHalls() const { return halls; }
vector<Hall>& HallManager::getHalls() { return halls; }

// ── Utility ───────────────────────────────────────────────────────────────────
bool HallManager::isEmpty()   const { return halls.empty(); }
int  HallManager::hallCount() const { return static_cast<int>(halls.size()); }

void HallManager::resetAllAssignments() {
    for (auto& h : halls)
        for (auto& r : h.getRooms())
            r.resetAssignments();
}
