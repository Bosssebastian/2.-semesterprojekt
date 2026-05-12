#include "database.h"
#include <sqlite3.h>
#include <iostream>

DataBase::DataBase(const std::string& file) {
    int return_value = sqlite3_open(file.c_str(), &db); 
    if(return_value) {
        std::cerr << "Error, could not open database. " << sqlite3_errmsg(db) << std::endl;
    }   
}

