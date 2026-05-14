#include "database.h"
#include <iostream>

int main() {
    DataBase sql("test.db"); // input filename insdie "" 

    std::string current_state_storage =
        "CREATE TABLE current_state_storage ("
        "storage_id INTEGER PRIMARY KEY, "
        "object_id INTEGER NOT NULL, "
        "occupied INTEGER NOT NULL "

        ");";
    
    std::string current_state_vision = 
        "CREATE TABLE current_state_vision ("
        "object TEXT NOT NULL, "
        "size INTEGER NOT NULL, "
        "color TEXT NOT NULL"
        ");";

    std::string history = 
        "CREATE TABLE history ("
        "object TEXT NOT NULL, "
        "slot INTEGER NOT NULL, "
        "time_stamp REAL NOT NULL"
        ");";
    
    sql.execute(current_state_storage);
    sql.execute(current_state_vision);
    sql.execute(history);

    
    return 0;
}