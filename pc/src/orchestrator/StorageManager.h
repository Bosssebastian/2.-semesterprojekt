#pragma once
#include <vector>


class StorageManager {
public:
    StorageManager(int slotCount = 8);

    bool hasFreeSlot() const;
    int getFreeSlot() const;
    void occupySlot(int slotIndex);
    void freeSlot(int slotIndex);
    std::vector<bool> getSlotStates() const;

private:
    const int slotCount;
    std::vector<bool> slotStates; // true if occupied, false if free
    
};
