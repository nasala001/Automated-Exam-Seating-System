#include <iostream>
#include "HallManager.h"
#include "HallRepository.h"

using namespace std;

// ─────────────────────────────────────────────────────────────────────────────
//  main.cpp – application entry point
//  Demonstrates: Hall/Room creation -> SQLite save -> SQLite load back
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    // ── Open (or create) the database file ────────────────────────────────────
    sqlite3* db = HallRepository::openDatabase("exam_seating.db");
    if (!db) {
        cerr << "[FATAL] Could not open database.\n";
        return 1;
    }
    HallRepository hallRepo(db);

    // ── Build halls/rooms in memory (same as before) ──────────────────────────
    HallManager manager;
    try {
        Hall hall1("TTC", "TTC Hall");
        hall1.addRoom(Room("R101", "TTC Main Room",       5,  8));
        hall1.addRoom(Room("R102", "TTC Isolated Room",   2,  5, true,  false));
        hall1.addRoom(Room("R103", "TTC Accessible Room", 3,  6, false, true));
        manager.addHall(hall1);

        Hall hall2("MPH", "Multipurpose Hall");
        hall2.addRoom(Room("R201", "MPH Main Room",       10, 20));
        hall2.addRoom(Room("R202", "MPH Accessible Room", 3,  5, false, true));
        manager.addHall(hall2);
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        sqlite3_close(db);
        return 1;
    }

    // ── Save each hall (with its rooms) to SQLite, skipping duplicates ────────
    cout << "[DB] Saving halls...\n";
    for (const auto& hall : manager.getHalls()) {
        if (hallRepo.exists(hall.getHallID())) {
            cout << "  -> " << hall.getHallID() << " already in DB, skipping.\n";
            continue;
        }
        if (hallRepo.save(hall))
            cout << "  -> Saved " << hall.getHallID() << " (" << hall.getRoomCount() << " rooms)\n";
        else
            cout << "  -> FAILED to save " << hall.getHallID() << "\n";
    }

    // ── Load everything back from SQLite to prove persistence works ──────────
    cout << "\n[DB] Loading halls back from database...\n";
    vector<Hall> loadedHalls = hallRepo.loadAll();
    for (const auto& hall : loadedHalls) {
        cout << "  Hall: " << hall.getHallID() << " - " << hall.getHallName()
             << " | Rooms: " << hall.getRoomCount()
             << " | Capacity: " << hall.getTotalCapacity() << "\n";
        for (const auto& room : hall.getRooms())
            cout << "    -> " << room.getRoomID() << " " << room.getBadge()
                 << " cap=" << room.getCapacity() << "\n";
    }

    sqlite3_close(db);
    return 0;
}
