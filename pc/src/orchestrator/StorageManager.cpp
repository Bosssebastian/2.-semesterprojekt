#include "StorageManager.h"
#include "database.h"
StorageManager::StorageManager(int slotCount) : slotCount(slotCount), slotStates(slotCount, false) {
}


bool StorageManager::hasFreeSlot() const {
        "SELECT COUNT(*) FROM current_state_storage, "
        "WHERE occupied = 0;";
    
    return false;
}

int StorageManager::getFreeSlot() const {
    "SELECT slot_id FROM current_state_Storage, "
    "WHERE occupied = 0, "
    "LIMIT 1;";
}

void StorageManager::occupySlot(int slotIndex) {
    "UPDATE current_state_storage, "
    "SET occupied = 1, "
    "object_id = ?, "
    "WHERE slot_id = ?;";
}

void StorageManager::freeSlot(int slotIndex) {
    "UPDATE current_state_storage, "
    "SET occupied = 0, "
    "object_id = ?, "
    "WHERE slot_id = ?;";
}

std::vector<bool> StorageManager::getSlotStates() const {
    return slotStates;
}