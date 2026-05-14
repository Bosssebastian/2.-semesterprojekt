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