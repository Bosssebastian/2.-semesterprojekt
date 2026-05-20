#include "StorageManager.h"
#include "database.h"
#include <iostream>

StorageManager::StorageManager(DataBase& db) : mDatabase(db), slotStates(8, false) {
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

std::vector<bool> StorageManager::getSlotStates(){
    //for(int i = 0; i < 8; i++){
    std::string sql = 
        "SELECT FROM current_state_storage * occupied; ";

    sqlite3_stmt* stmt; 
    sqlite3_prepare_v2(mDatabase.getHandle(), sql.c_str(), -1, &stmt, nullptr);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << sqlite3_column_int(stmt,0) << "\n";
    }

    sqlite3_finalize(stmt);
    //}
    return slotStates;
}
bool StorageManager::isOccupied(int slotIndex){
    return true;
}
