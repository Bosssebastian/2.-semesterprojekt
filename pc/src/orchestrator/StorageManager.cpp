#include "StorageManager.h"
#include "database.h"
StorageManager::StorageManager(DataBase& db) : mDatabase(db) {
    sqlite3_exec(mDatabase.getHandle(), "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);//enable foreing keys
}


bool StorageManager::hasFreeSlot() const {
    std::string sql = 
        "SELECT COUNT(*) FROM current_state_storage WHERE occupied = 0;";
    
    sqlite3_stmt* stmt; 
    sqlite3_prepare_v2(mDatabase.getHandle(), sql.c_str(), -1, &stmt, nullptr);

    int count = 0;
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0;
}

int StorageManager::getFreeSlot() const {
    std::string sql = 
        "SELECT slot_id FROM current_state_storage WHERE occupied = 0 LIMIT 1;";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(mDatabase.getHandle(), sql.c_str(), -1, &stmt, nullptr);

    int slot = -1;
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        slot = sqlite3_column_int(stmt,0);
    }
    sqlite3_finalize(stmt);
    return slot;
}

void StorageManager::occupySlot(int slotIndex, int objectId) {
    std::string sql = 
        "UPDATE current_state_storage SET occupied = 1, "
        "object_id = " + std::to_string(objectId) + " WHERE slot_id = " + std::to_string(slotIndex) + ";";
    
    mDatabase.execute(sql);
}


void StorageManager::freeSlot(int slotIndex) {
std::string sql = 
        "UPDATE current_state_storage SET occupied = 0, "
        "object_id = NULL WHERE slot_id = " + std::to_string(slotIndex) + ";";
    
    mDatabase.execute(sql);
}

std::vector<bool> StorageManager::getSlotStates() const {
    std::vector<bool> states(8, false);

    std::string sql =
        "SELECT slot_id, occupied FROM current_state_storage;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(mDatabase.getHandle(), sql.c_str(), -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int slot = sqlite3_column_int(stmt, 0);
        int occupied = sqlite3_column_int(stmt, 1);

        if (slot >= 0 && slot < 8) {
            states[slot] = occupied;
        }
    }

    sqlite3_finalize(stmt);

    return states;
}

bool StorageManager::isOccupied(int slotIndex) const {
    std::string sql =
        "SELECT occupied FROM current_state_storage WHERE slot_id = "
        + std::to_string(slotIndex) + ";";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(mDatabase.getHandle(), sql.c_str(), -1, &stmt, nullptr);

    bool occupied = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        occupied = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return occupied;
}
