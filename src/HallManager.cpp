#include "HallManager.h"
#include <iostream>
#include <stdexcept>

using namespace std;

void HallManager::addHall(const Hall& hall) {
    for (const auto& h : halls)
        if (h.hallGetID() == hall.hallGetID())
            throw invalid_argument("[ERROR] Hall ID '" + hall.hallGetID() + "' already exists.");
    halls.push_back(hall);
}

bool HallManager::removeHall(const string& hallID) {
    for (auto it = halls.begin(); it != halls.end(); ++it) {
        if (it->hallGetID() == hallID) { halls.erase(it); return true; }
    }
    return false;
}

Hall* HallManager::findHall(const string& hallID) {
    for (auto& h : halls)
        if (h.hallGetID() == hallID) return &h;
    return nullptr;
}

Hall* HallManager::findHallByName(const string& name) {
    for (auto& h : halls)
        if (h.getHallName() == name) return &h;
    return nullptr;
}

bool HallManager::editHallName(const string& hallID, const string& newName) {
    Hall* h = findHall(hallID);
    if (!h) return false;
    h->setHallName(newName);
    return true;
}

int HallManager::getTotalSystemCapacity() const {
    int t = 0; for (const auto& h : halls) t += h.getTotalCapacity(); return t;
}

void HallManager::checkCapacityWarning(int totalStudents) const {
    int seats = getTotalSystemCapacity();
    if (totalStudents > seats)
        cout << "[WARNING] Students exceed seats by " << (totalStudents - seats) << "!\n";
    else
        cout << "[OK] " << (seats - totalStudents) << " seats remaining.\n";
}

const vector<Hall>& HallManager::getHalls() const { return halls; }