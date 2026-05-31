
#include <iostream>
#include "HallManager.h"

using namespace std;

int main() {
    HallManager manager;

    try {
        Hall hall1("TTC", "TTC Hall");
        hall1.addRoom(Room("R101", "TTC Room 1", 5, 8));
        hall1.addRoom(Room("R102", "TTC Isolated Room", 2, 5, true, false));
        manager.addHall(hall1);

        Hall hall2("MPH", "Multipurpose Hall");
        hall2.addRoom(Room("R201", "MPH Main Room", 10, 20));
        hall2.addRoom(Room("R202", "MPH Accessible Room", 3, 5, false, true));
        manager.addHall(hall2);
    } catch (const exception& e) {
        cout << e.what() << "\n";
    }

    // Safe pointer usage - ALWAYS check for null
    Hall* h = manager.findHall("TTC");
    if (h) {
        if (h->editRoom("R101", "TTC Main Exam Room", 6, 10))
            cout << "[OK] Room edited.\n";

        Room* r = h->findRoom("R101");
        if (r)
            cout << "Status: " << r->getStatusString()
                 << "  Occupancy: " << r->getOccupancyPercent() << "%\n";
    }

    manager.checkCapacityWarning(250);
    manager.checkCapacityWarning(50);

    // Validation tests
    try { Room bad("", "", -1, 0); }
    catch (const exception& e) { cout << e.what() << "\n"; }

    try { manager.addHall(Hall("TTC", "Dup")); }
    catch (const exception& e) { cout << e.what() << "\n"; }

    return 0;
}