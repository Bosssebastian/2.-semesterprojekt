#include "database.h"
#include <sqlite3.h>
#include <iostream>

DataBase::DataBase(const std::string& file) {
    sqlite3_open("database.db", &db);    
}

