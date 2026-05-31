#ifndef HALLMANAGER_H
#define HALLMANAGER_H

#include <vector>
#include "Hall.h"

class HallManager {
private:
    std::vector<Hall> halls;

public:
    void  addHall(const Hall& hall);
    bool  removeHall(const std::string& hallID);
    Hall* findHall(const std::string& hallID);
    Hall* findHallByName(const std::string& name);
    bool  editHallName(const std::string& hallID, const std::string& newName);

    int  getTotalSystemCapacity() const;
    void checkCapacityWarning(int totalStudents) const;

    const std::vector<Hall>& getHalls() const;
};

#endif