#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include <sqlite3.h>
class DataBase {
private:
    sqlite3* db;
    std::string filename;

public:
    DataBase(const std::string& file);
    ~DataBase();

    void openDataBase();
    void closeDatabase();
};

#endif