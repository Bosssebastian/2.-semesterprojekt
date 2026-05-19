#include "database.h"
#include "StorageManager.h"
#include <sqlite3.h>
#include <iostream>
#include <chrono>

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
        std::cout << "Database closed \n";
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
        "slot_id INTEGER PRIMARY KEY, "
        "object_id INTEGER, "
        "occupied INTEGER NOT NULL"

        ");";
    
    std::string current_state_vision = 
        "CREATE TABLE IF NOT EXISTS current_state_vision ("
        "object_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "object TEXT NOT NULL, "
        "size TEXT NOT NULL, "
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

    for(int i = 0; i < 8; ++i) {
        std::string sql = 
        "INSERT OR IGNORE INTO current_state_storage "
        "(slot_id, object_id, occupied) "
        "VALUES(" + std::to_string(i) + ", NULL, 0);";
    execute(sql);
    };
    
}

int DataBase::insertVisionObject(const std::string& object, std::string size, const std::string& color) { 
    std::cout << "object = " << object << " size = " << size << " color = " << color << std::endl;
    std::string sql = 
    "INSERT INTO current_state_vision (object, size, color) VALUES ('"+ object +"' , '" + size + "', '" + color + "');";

    if(execute(sql)) {
        return sqlite3_last_insert_rowid(db);
    };

    return -1;
}

void DataBase::updateStorageSlot(int slotId, int objectId, bool occupied) {
    std::string sql =
        "UPDATE current_state_storage SET "
        "object_id = " + std::to_string(objectId) + ", "
        "occupied = " + std::to_string(occupied) +" "
        "WHERE slot_id = " + std::to_string(slotId) + ";";

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

//getting the time

double DataBase::time() {
    using namespace std::chrono;
    return duration<double>(system_clock::now().time_since_epoch()).count();
}
