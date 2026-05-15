#include "database.h"
#include <iostream>

int main() {
    DataBase sql("test.db"); // input filename inside "" 
    sql.createTables();
    sql.updateStorageSlot();
    sql.insertVisionObject();
    sql.insertHistory();

    
    return 0;
}