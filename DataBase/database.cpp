#include "database.h"
#include <sqlite3.h>
#include <iostream>

DataBase::DataBase(const std::string& file) : filename(file), db(nullptr) {

    int return_value = sqlite3_open(file.c_str(), &db); 
    
    if(return_value) {
        std::cerr << "Error, could not open database. " << sqlite3_errmsg(db) << std::endl;
    }   
    else {
        std::cout << "Opened database succesfully! " << std::endl;
    }
}

DataBase::~DataBase() {
    if(db) {
        sqlite3_close(db);
    }
}

bool DataBase:: execute(const std::string& sql) {
    char* errMsg = nullptr;

    int return_code = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg); 

    if(return_code != SQLITE_OK) {
        std::cerr << " SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

void DataBase::createTables() {
     std::string current_state_storage =
        "CREATE TABLE IF NOT EXISTS current_state_storage ("
        "storage_id INTEGER PRIMARY KEY, "
        "object_id INTEGER, "
        "occupied INTEGER NOT NULL"

        ");";
    
    std::string current_state_vision = 
        "CREATE TABLE IF NOT EXISTS current_state_vision ("
        "object_id INTEGER PRIMARY KEY, "
        "object TEXT NOT NULL, "
        "size INTEGER NOT NULL, "
        "color TEXT NOT NULL"
        ");";

    std::string history = 
        "CREATE TABLE IF NOT EXISTS history ("
        "object_id INTEGER, "
        "slot INTEGER NOT NULL, "
        "time_stamp REAL NOT NULL"
        ");";
    
    execute(current_state_storage);
    execute(current_state_vision);
    execute(history);
}

void DataBase::insertVisionObject(const std::string& object, int size, const std::string& color) {
    std::string sql = 
    "INSERT INTO current_state_vision (object, size, color) VALUES ('"+ object +"' , " + std::to_string(size) + ", '" + color + "');";
    execute(sql);
}

void DataBase::updateStorageSlot(int storageId, int objectId, int occupied) {
    std::string sql =
        "UPDATE current_state_storage SET "
        "object_id = " + std::to_string(objectId) + ", "
        "occupied = " + std::to_string(occupied) +
        " WHERE storage_id = " + std::to_string(storageId) + ";";

    execute(sql);
}

void DataBase::insertHistory(int objectId, int slot, double timestamp) {
    std::string sql =
        "INSERT INTO history (object_id, slot, time_stamp) VALUES ("
        + std::to_string(objectId) + ", "
        + std::to_string(slot) + ", "
        + std::to_string(timestamp) + ");";

    execute(sql);
}