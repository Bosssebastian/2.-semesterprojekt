#pragma once
#include <vector>
#include "database.h"


class StorageManager {
public:
    StorageManager(DataBase& db);

    bool hasFreeSlot() const;
    int getFreeSlot() const;
    void occupySlot(int slotIndex, int objectId);
    void freeSlot(int slotIndex);

    std::vector<bool> getSlotStates();

    std::vector<bool> getSlotStates() const;
    bool isOccupied(int slotIndex) const;

private:

    DataBase& mDatabase; 
    


};
