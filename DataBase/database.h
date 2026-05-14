#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include <sqlite3.h>
class DataBase {
private:
    sqlite3* db;
    std::string filename; //the file which is created when database is opened

public:
    DataBase(const std::string& file); //opens database
    ~DataBase(); // closes database

    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    bool execute(const std::string& sql);

};

#endif