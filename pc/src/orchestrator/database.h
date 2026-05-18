#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include <sqlite3.h>
class DataBase {
private:
    sqlite3* db;
    std::string filename; //the file which is created when database is opened
    double timestamp{0};

public:
    DataBase(const std::string& file); //opens database
    ~DataBase(); // closes database

    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    bool execute(const std::string& sql);
    void createTables();
    void insertVisionObject(const std::string& object, std::string size, const std::string& color); 
    void updateStorageSlot(int slotId, int objectId, bool occupied);
    void insertHistory(int objectId, int slot, double timestamp);

    double time();

    
};

#endif