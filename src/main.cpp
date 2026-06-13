#include <iostream>
#include "HallManager.h"

using namespace std;

// ─────────────────────────────────────────────────────────────────────────────
//  main.cpp  – application entry point
//
//  Currently: smoke-tests the Hall/Room layer.
//  Later:     will initialize Qt UI, load SQLite DB, and launch the app.
//
//  NOTE: All validation/unit tests belong in tests/unit/ with GoogleTest.
//        This file should only contain startup logic once tests are added.
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    HallManager manager;

    // ── Setup: add halls and rooms ────────────────────────────────────────────
    try {
        Hall hall1("TTC", "TTC Hall");
        hall1.addRoom(Room("R101", "TTC Main Room", 5, 8));
        hall1.addRoom(Room("R102", "TTC Isolated Room", 2, 5, true, false));
        hall1.addRoom(Room("R103", "TTC Accessible Room", 3, 6, false, true));
        manager.addHall(hall1);

        Hall hall2("MPH", "Multipurpose Hall");
        hall2.addRoom(Room("R201", "MPH Main Room", 10, 20));
        hall2.addRoom(Room("R202", "MPH Accessible Room", 3, 5, false, true));
        manager.addHall(hall2);

    }
    catch (const exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    // ── Edit a room ───────────────────────────────────────────────────────────
    Hall* ttc = manager.findHall("TTC");
    if (ttc) {
        if (ttc->editRoom("R101", "TTC Main Exam Room", 6, 10))
            cout << "[OK] Room R101 edited.\n";

        Room* r = ttc->findRoom("R101");
        if (r) {
            cout << "  Room : " << r->getRoomName() << "\n"
                << "  Badge: " << r->getBadge() << "\n"
                << "  Cap  : " << r->getCapacity() << "\n"
                << "  Status: " << r->getStatusString() << "\n";
        }
    }

    // ── Capacity check ────────────────────────────────────────────────────────
    cout << "\n[System] Total capacity : " << manager.getTotalSystemCapacity() << "\n";
    cout << "[System] Total available: " << manager.getTotalSystemAvailable() << "\n";

    if (!manager.checkCapacity(250))
        cout << "[WARNING] 250 students exceed available seats!\n";
    else
        cout << "[OK] Enough seats for 250 students.\n";

    // ── Engine-facing query demo ──────────────────────────────────────────────
    auto accessibleRooms = manager.getAllAccessibleRooms();
    cout << "\n[Query] Accessible rooms available: " << accessibleRooms.size() << "\n";
    for (auto* r : accessibleRooms)
        cout << "  -> " << r->getRoomID() << " | " << r->getRoomName()
        << " | Cap: " << r->getCapacity() << "\n";

    auto isolatedRooms = manager.getAllIsolatedRooms();
    cout << "\n[Query] Isolated rooms available: " << isolatedRooms.size() << "\n";
    for (auto* r : isolatedRooms)
        cout << "  -> " << r->getRoomID() << " | " << r->getRoomName()
        << " | Cap: " << r->getCapacity() << "\n";

    // ── SQLite serialisation preview ──────────────────────────────────────────
    cout << "\n[SQL Preview] Hall INSERT values:\n";
    for (const auto& h : manager.getHalls())
        cout << "  (" << h.toSQLValues() << ")\n";

    cout << "\n[SQL Preview] Room INSERT values:\n";
    for (const auto& h : manager.getHalls())
        for (const auto& r : h.getRooms())
            cout << "  (" << r.toSQLValues() << ")\n";

    return 0;
}
