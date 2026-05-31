#ifndef ROOM_H
#define ROOM_H

#include <string>

enum class RoomStatus { AVAILABLE, ALMOST_FULL, FULL };

class Room {
private:
    std::string roomID;
    std::string roomName;
    int capacity;
    int rows;
    int columns;
    bool isIsolated;
    bool isAccessible;
    int assignedStudents;

public:
    Room(std::string id, std::string name, int rows, int cols,
         bool isolated = false, bool accessible = false);

    // Getters
    std::string getRoomID()       const;
    std::string getRoomName()     const;
    int         getCapacity()     const;
    int         getRows()         const;
    int         getColumns()      const;
    bool        getIsIsolated()   const;
    bool        getIsAccessible() const;
    int         getAssigned()     const;
    int         getAvailable()    const;
    double      getOccupancyPercent() const;

    // Status / badge (Qt uses these for colors)
    RoomStatus  getStatus()  const;
    std::string getBadge()   const;
    std::string getStatusString() const; // "AVAILABLE" / "ALMOST FULL" / "FULL"

    // Setters
    void setRoomName(const std::string& name);
    void setIsolated(bool val);
    void setAccessible(bool val);
    void setDimensions(int r, int c); // FIXED: checks assigned count

    // Seat management
    bool hasAvailableSeat() const;
    bool assignSeat();
    void removeSeat();
    void resetAssignments();
};

#endif