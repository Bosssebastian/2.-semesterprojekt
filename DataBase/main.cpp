#include "database.h"
#include <iostream>

int main() {
    DataBase sql("test.db"); // input filename insdie "" 

    std::string current_state_storage =
        "CREATE TABLE current_state_storage ("
        "storage_id INTEGER PRIMARY KEY, "
        "object TEXT NOT NULL, "
        "occupied INTEGER NOT NULL, "

        ");";
    
        std::string current_state_vision = 
            "CREATE TABLE current_state_vision ("

            ");";

    std::string history = 
        "CREATE TABLE history ("

        ");";
    
    sql.execute(current_state_storage);
    sql.execute(current_state_vision);
    sql.execute(history);

    
    return 0;
}