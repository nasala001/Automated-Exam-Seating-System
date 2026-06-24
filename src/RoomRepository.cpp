#include "RoomRepository.h"
#include <iostream>

using namespace std;

RoomRepository::RoomRepository(sqlite3* db) : db(db) {}

// ── save ─────────────────────────────────────────────────────────────────────
bool RoomRepository::save(const Room& room, const string& hallID) {
    const char* sql =
        "INSERT INTO rooms (room_id, room_name, capacity, rows, columns, "
        "is_isolated, is_accessible, hall_id_fk) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[RoomRepository] Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, room.getRoomID().c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, room.getRoomName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 3, room.getCapacity());
    sqlite3_bind_int (stmt, 4, room.getRows());
    sqlite3_bind_int (stmt, 5, room.getColumns());
    sqlite3_bind_int (stmt, 6, room.isIsolated()   ? 1 : 0);
    sqlite3_bind_int (stmt, 7, room.isAccessible() ? 1 : 0);
    sqlite3_bind_text(stmt, 8, hallID.c_str(),             -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) cerr << "[RoomRepository] Insert failed: " << sqlite3_errmsg(db) << "\n";

    sqlite3_finalize(stmt);
    return ok;
}

// ── loadByHallID ─────────────────────────────────────────────────────────────
vector<Room> RoomRepository::loadByHallID(const string& hallID) {
    vector<Room> result;
    const char* sql =
        "SELECT room_id, room_name, rows, columns, is_isolated, is_accessible "
        "FROM rooms WHERE hall_id_fk = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[RoomRepository] Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return result;
    }

    sqlite3_bind_text(stmt, 1, hallID.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string roomID   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string roomName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int    rows     = sqlite3_column_int(stmt, 2);
        int    cols     = sqlite3_column_int(stmt, 3);
        bool   isolated = sqlite3_column_int(stmt, 4) == 1;
        bool   accessible = sqlite3_column_int(stmt, 5) == 1;

        try {
            result.emplace_back(roomID, roomName, rows, cols, isolated, accessible);
        } catch (const exception& e) {
            cerr << "[RoomRepository] Skipped invalid row: " << e.what() << "\n";
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

// ── update ───────────────────────────────────────────────────────────────────
bool RoomRepository::update(const Room& room) {
    const char* sql =
        "UPDATE rooms SET room_name = ?, capacity = ?, rows = ?, columns = ?, "
        "is_isolated = ?, is_accessible = ? WHERE room_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[RoomRepository] Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, room.getRoomName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, room.getCapacity());
    sqlite3_bind_int (stmt, 3, room.getRows());
    sqlite3_bind_int (stmt, 4, room.getColumns());
    sqlite3_bind_int (stmt, 5, room.isIsolated()   ? 1 : 0);
    sqlite3_bind_int (stmt, 6, room.isAccessible() ? 1 : 0);
    sqlite3_bind_text(stmt, 7, room.getRoomID().c_str(),   -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) cerr << "[RoomRepository] Update failed: " << sqlite3_errmsg(db) << "\n";

    sqlite3_finalize(stmt);
    return ok;
}

// ── remove ───────────────────────────────────────────────────────────────────
bool RoomRepository::remove(const string& roomID) {
    const char* sql = "DELETE FROM rooms WHERE room_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[RoomRepository] Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, roomID.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

// ── exists ───────────────────────────────────────────────────────────────────
bool RoomRepository::exists(const string& roomID) {
    const char* sql = "SELECT 1 FROM rooms WHERE room_id = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, roomID.c_str(), -1, SQLITE_TRANSIENT);
    bool found = sqlite3_step(stmt) == SQLITE_ROW;

    sqlite3_finalize(stmt);
    return found;
}
