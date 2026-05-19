#pragma once
#include <vector>


class StorageManager {
public:
    StorageManager(DataBase& db);

    bool hasFreeSlot() const;
    int getFreeSlot() const;
    void occupySlot(int slotIndex, int objectId);
    void freeSlot(int slotIndex);

private:
    DataBase& mDatabase; 
    
};
