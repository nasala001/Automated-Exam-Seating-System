#ifndef ROOM_H

#define ROOM_H



#include <string>

namespace nasala {


// ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ
//  RoomStatus  ÔÇô occupancy state of a room
// ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇ

enum class RoomStatus {
    AVAILABLE,    // < 80 % full
    ALMOST_FULL,  // >= 80 % full
    FULL          // 100 % full
};

// ─────────────────────────────────────────────────────────────────────────────
//  Room  – one physical exam room inside a Hall
//
//  Responsibilities (your module only):
//    • Store physical attributes (rows, cols, flags)
//    • Track assigned vs available seats
//    • Provide queries the allocation engine needs
//    • Serialize itself for SQLite persistence
// ─────────────────────────────────────────────────────────────────────────────
class Room {
public:
    // ── Construction ──────────────────────────
    Room(std::string id,
         std::string name,
         int         rows,
         int         cols,
         bool        isolated   = false,
         bool        accessible = false);

    // ── Identity getters ──────────────────────
    const std::string& getRoomID()   const;
    const std::string& getRoomName() const;

    // ── Physical attribute getters ────────────
    int  getCapacity()  const;   // rows * cols
    int  getRows()      const;
    int  getColumns()   const;

    // ── Flag getters (no "get" prefix for bools)
    bool isIsolated()   const;
    bool isAccessible() const;

    // ── Occupancy getters ─────────────────────
    int    getAssigned()         const;
    int    getAvailable()        const;
    double getOccupancyPercent() const;

    // ── Status / badge (used by UI for color coding)
    RoomStatus  getStatus()       const;
    std::string getStatusString() const;  // "AVAILABLE" | "ALMOST FULL" | "FULL"
    std::string getBadge()        const;  // "[ISOLATED] [ACCESSIBLE]"

    // ── Setters (with validation) ─────────────
    void setRoomName  (const std::string& name);
    void setIsolated  (bool val);
    void setAccessible(bool val);
    void setDimensions(int rows, int cols);   // guards against shrinking below assigned

    // ── Seat management ───────────────────────
    bool hasAvailableSeat()  const;
    bool assignSeat();          // returns false if full
    void removeSeat();          // guards against going below 0
    void resetAssignments();    // sets assignedStudents = 0

    // ── SQLite serialisation helpers ──────────
    // Returns a comma-separated value string for INSERT:
    //   room_id, room_name, capacity, rows, columns, is_isolated, is_accessible
    std::string toSQLValues() const;

    // ── Equality (used by HallManager duplicate check)
    bool operator==(const Room& other) const;

private:
    std::string roomID;
    std::string roomName;
    int         capacity;          // always rows * columns
    int         rows;
    int         columns;
    bool        isolated;
    bool        accessible;
    int         assignedStudents;
};







} // namespace nasala

#endif // ROOM_H
