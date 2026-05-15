#include "database.h"
#include <iostream>

int main() {
    DataBase sql("test.db"); // input filename inside "" 
    sql.createTables();
    sql.updateStorageSlot(1, 4, 1); // storage id, object id, occupied state
    sql.insertVisionObject("cube", 5, "red"); //object, size, color
    sql.insertHistory(4,1, 12.33); // object id, slot, timestamp (double)

    
    return 0;
}