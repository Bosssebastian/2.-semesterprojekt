#pragma once

enum class OrchestratorState {
    Starting,
    Idle,
    InStor_StorageMoveToSlot,

    InStor_RobotOverInput,

    InStor_RobotToCube,

    InStor_GripperClose,

    InStor_RobotOverStorage,

    InStor_StorageWaitingOnMove,

    InStor_RobotDownToSlot,

    InStor_GripperOpen,

    InStor_RobotUpFromSlot,

    InStor_Complete,
    Resetting,
    Stopping,
    Stopped,
    Faulted
};

inline const char* toString(OrchestratorState state) {
    switch (state) {
        case OrchestratorState::Starting:
            return "Starting";
        case OrchestratorState::Idle:
            return "Idle";
        case OrchestratorState::InStor_StorageMoveToSlot:
            return "InStor_StorageMoveToSlot";
        case OrchestratorState::InStor_RobotOverInput:
            return "InStor_RobotOverInput";
        case OrchestratorState::InStor_RobotToCube:
            return "InStor_RobotToCube";
        case OrchestratorState::InStor_GripperClose:
            return "InStor_GripperClose";
        case OrchestratorState::InStor_RobotOverStorage:
            return "InStor_RobotOverStorage";
        case OrchestratorState::InStor_StorageWaitingOnMove:
            return "InStor_StorageWaitingOnMove";
        case OrchestratorState::InStor_RobotDownToSlot:
            return "InStor_RobotDownToSlot";
        case OrchestratorState::InStor_GripperOpen:
            return "InStor_GripperOpen";
        case OrchestratorState::InStor_RobotUpFromSlot:
            return "InStor_RobotUpFromSlot";
        case OrchestratorState::InStor_Complete:
            return "InStor_Complete";
        case OrchestratorState::Resetting:
            return "Resetting";
        case OrchestratorState::Stopping:
            return "Stopping";
        case OrchestratorState::Stopped:
            return "Stopped";
        case OrchestratorState::Faulted:
            return "Faulted";
    }

    return "Unknown";
}

inline const char* toUiString(OrchestratorState state) {
    switch (state) {
        case OrchestratorState::Starting:
            return "Starting up";
        case OrchestratorState::Idle:
            return "Waiting for input";
        case OrchestratorState::InStor_StorageMoveToSlot:
            return "Storage move to slot";
        case OrchestratorState::InStor_RobotOverInput:
            return "Moving robot over input";
        case OrchestratorState::InStor_RobotToCube:
            return "Moving robot to cube";
        case OrchestratorState::InStor_GripperClose:
            return "Closing gripper";
        case OrchestratorState::InStor_RobotOverStorage:
            return "Moving robot over storage";
        case OrchestratorState::InStor_StorageWaitingOnMove:
            return "Waiting on Storage to move to position";
        case OrchestratorState::InStor_RobotDownToSlot:
            return "Robot down to slot";
        case OrchestratorState::InStor_GripperOpen:
            return "Opening gripper";
        case OrchestratorState::InStor_RobotUpFromSlot:
            return "Robot up from slot";
        case OrchestratorState::InStor_Complete:
            return "Input-to-storage complete";
        case OrchestratorState::Resetting:
            return "Resetting system";
        case OrchestratorState::Stopping:
            return "Stopping system";
        case OrchestratorState::Stopped:
            return "Stopped";
        case OrchestratorState::Faulted:
            return "Fault";
    }

    return "Unknown";
}
