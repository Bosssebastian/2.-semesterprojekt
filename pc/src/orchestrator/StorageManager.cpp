#include "StorageManager.h"
#include "database.h"
StorageManager::StorageManager(int slotCount) : slotCount(slotCount), slotStates(slotCount, false) {
}


bool StorageManager::hasFreeSlot() const {
    for (int i = 0; i < slotCount; ++i) {
        if (!slotStates[i]) {
            return true;
        }
    }
    return false;
}

int StorageManager::getFreeSlot() const {
    for (int i = 0; i < slotCount; ++i) {
        if (!slotStates[i]) {
            return i;
        }
    }
    return -1;
}

void StorageManager::occupySlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < slotCount) {
        slotStates[slotIndex] = true;
    }
}

void StorageManager::freeSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < slotCount) {
        slotStates[slotIndex] = false;
    }
}