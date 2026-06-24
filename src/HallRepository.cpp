#include "HallRepository.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

HallRepository::HallRepository(sqlite3* db) : db(db), roomRepo(db) {}

// ── openDatabase ──────────────────────────────────────────────────────────────
// Opens exam_seating.db and executes data/schema.sql to create tables.
sqlite3* HallRepository::openDatabase(const string& dbPath) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        cerr << "[HallRepository] Cannot open DB: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return nullptr;
    }

    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    // Inline schema — avoids depending on schema.sql being shipped alongside the exe.
    const char* schema =
        "CREATE TABLE IF NOT EXISTS halls ("
        "  id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  hall_id   TEXT NOT NULL UNIQUE,"
        "  hall_name TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS rooms ("
        "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  room_id       TEXT NOT NULL UNIQUE,"
        "  room_name     TEXT NOT NULL,"
        "  capacity      INTEGER NOT NULL CHECK (capacity > 0),"
        "  rows          INTEGER NOT NULL CHECK (rows > 0),"
        "  columns       INTEGER NOT NULL CHECK (columns > 0),"
        "  is_isolated   INTEGER NOT NULL DEFAULT 0,"
        "  is_accessible INTEGER NOT NULL DEFAULT 0,"
        "  hall_id_fk    TEXT NOT NULL,"
        "  FOREIGN KEY (hall_id_fk) REFERENCES halls(hall_id) ON DELETE CASCADE"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_rooms_hall_id ON rooms(hall_id_fk);";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, schema, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "[HallRepository] Schema creation failed: " << errMsg << "\n";
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return nullptr;
    }

    return db;
}

// ── save ─────────────────────────────────────────────────────────────────────
bool HallRepository::save(const Hall& hall) {
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* sql = "INSERT INTO halls (hall_id, hall_name) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[HallRepository] Prepare failed: " << sqlite3_errmsg(db) << "\n";
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    sqlite3_bind_text(stmt, 1, hall.getHallID().c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hall.getHallName().c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    if (!ok) {
        cerr << "[HallRepository] Hall insert failed: " << sqlite3_errmsg(db) << "\n";
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    // Save every room belonging to this hall, in the same transaction.
    for (const auto& room : hall.getRooms()) {
        if (!roomRepo.save(room, hall.getHallID())) {
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }
    }

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

// ── loadAll ──────────────────────────────────────────────────────────────────
vector<Hall> HallRepository::loadAll() {
    vector<Hall> result;
    const char* sql = "SELECT hall_id, hall_name FROM halls;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[HallRepository] Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return result;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string hallID   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string hallName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        try {
            Hall hall(hallID, hallName);
            for (auto& room : roomRepo.loadByHallID(hallID))
                hall.addRoom(room);
            result.push_back(move(hall));
        } catch (const exception& e) {
            cerr << "[HallRepository] Skipped invalid hall row: " << e.what() << "\n";
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

// ── loadByID ─────────────────────────────────────────────────────────────────
bool HallRepository::loadByID(const string& hallID, Hall& outHall) {
    const char* sql = "SELECT hall_id, hall_name FROM halls WHERE hall_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, hallID.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        string id   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        try {
            Hall hall(id, name);
            for (auto& room : roomRepo.loadByHallID(id))
                hall.addRoom(room);
            outHall = move(hall);
            found = true;
        } catch (const exception& e) {
            cerr << "[HallRepository] Failed to build hall: " << e.what() << "\n";
        }
    }

    sqlite3_finalize(stmt);
    return found;
}

// ── update ───────────────────────────────────────────────────────────────────
bool HallRepository::update(const Hall& hall) {
    const char* sql = "UPDATE halls SET hall_name = ? WHERE hall_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[HallRepository] Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, hall.getHallName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hall.getHallID().c_str(),   -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

// ── remove ───────────────────────────────────────────────────────────────────
// Rooms are deleted automatically via ON DELETE CASCADE.
bool HallRepository::remove(const string& hallID) {
    const char* sql = "DELETE FROM halls WHERE hall_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[HallRepository] Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, hallID.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

// ── exists ───────────────────────────────────────────────────────────────────
bool HallRepository::exists(const string& hallID) {
    const char* sql = "SELECT 1 FROM halls WHERE hall_id = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, hallID.c_str(), -1, SQLITE_TRANSIENT);
    bool found = sqlite3_step(stmt) == SQLITE_ROW;

    sqlite3_finalize(stmt);
    return found;
}
