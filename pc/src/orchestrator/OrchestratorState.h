#pragma once

enum class OrchestratorState {
    Starting,
    Idle,
    InStor_StorageMoveToSlot_Cmd,

    InStor_RobotOverInput_Cmd,
    InStor_RobotOverInput_Wait,

    InStor_RobotToCube_Cmd,
    InStor_RobotToCube_Wait,

    InStor_GripperClose_Cmd,
    InStor_GripperClose_Wait,

    InStor_RobotOverStorage_Cmd,
    InStor_RobotOverStorage_Wait,

    InStor_StorageMoveToPos_Cmd,
    InStor_StorageMoveToPos_Wait,

    InStor_RobotDownToSlot_Cmd,
    InStor_RobotDownToSlot_Wait,

    InStor_GripperOpen_Cmd,
    InStor_GripperOpen_Wait,

    InStor_RobotUpFromSlot_Cmd,
    InStor_RobotUpFromSlot_Wait,

    InStor_Complete,
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
        case OrchestratorState::InStor_StorageMoveToSlot_Cmd:
            return "InStor_StorageMoveToSlot_Cmd";
        case OrchestratorState::InStor_RobotOverInput_Cmd:
            return "InStor_RobotOverInput_Cmd";
        case OrchestratorState::InStor_RobotOverInput_Wait:
            return "InStor_RobotOverInput_Wait";
        case OrchestratorState::InStor_RobotToCube_Cmd:
            return "InStor_RobotToCube_Cmd";
        case OrchestratorState::InStor_RobotToCube_Wait:
            return "InStor_RobotToCube_Wait";
        case OrchestratorState::InStor_GripperClose_Cmd:
            return "InStor_GripperClose_Cmd";
        case OrchestratorState::InStor_GripperClose_Wait:
            return "InStor_GripperClose_Wait";
        case OrchestratorState::InStor_RobotOverStorage_Cmd:
            return "InStor_RobotOverStorage_Cmd";
        case OrchestratorState::InStor_RobotOverStorage_Wait:
            return "InStor_RobotOverStorage_Wait";
        case OrchestratorState::InStor_StorageMoveToPos_Cmd:
            return "InStor_StorageMoveToPos_Cmd";
        case OrchestratorState::InStor_StorageMoveToPos_Wait:
            return "InStor_StorageMoveToPos_Wait";
        case OrchestratorState::InStor_RobotDownToSlot_Cmd:
            return "InStor_RobotDownToSlot_Cmd";
        case OrchestratorState::InStor_RobotDownToSlot_Wait:
            return "InStor_RobotDownToSlot_Wait";
        case OrchestratorState::InStor_GripperOpen_Cmd:
            return "InStor_GripperOpen_Cmd";
        case OrchestratorState::InStor_GripperOpen_Wait:
            return "InStor_GripperOpen_Wait";
        case OrchestratorState::InStor_RobotUpFromSlot_Cmd:
            return "InStor_RobotUpFromSlot_Cmd";
        case OrchestratorState::InStor_RobotUpFromSlot_Wait:
            return "InStor_RobotUpFromSlot_Wait";
        case OrchestratorState::InStor_Complete:
            return "InStor_Complete";
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
        case OrchestratorState::InStor_StorageMoveToSlot_Cmd:
            return "Storage move to slot";
        case OrchestratorState::InStor_RobotOverInput_Cmd:
            return "Moving robot over input";
        case OrchestratorState::InStor_RobotOverInput_Wait:
            return "Waiting for robot over input";
        case OrchestratorState::InStor_RobotToCube_Cmd:
            return "Moving robot to cube";
        case OrchestratorState::InStor_RobotToCube_Wait:
            return "Waiting for robot to cube";
        case OrchestratorState::InStor_GripperClose_Cmd:
            return "Closing gripper";
        case OrchestratorState::InStor_GripperClose_Wait:
            return "Waiting for gripper close";
        case OrchestratorState::InStor_RobotOverStorage_Cmd:
            return "Moving robot over storage";
        case OrchestratorState::InStor_RobotOverStorage_Wait:
            return "Waiting over storage";
        case OrchestratorState::InStor_StorageMoveToPos_Cmd:
            return "Storage move to position";
        case OrchestratorState::InStor_StorageMoveToPos_Wait:
            return "Waiting for storage move";
        case OrchestratorState::InStor_RobotDownToSlot_Cmd:
            return "Robot down to slot";
        case OrchestratorState::InStor_RobotDownToSlot_Wait:
            return "Waiting for robot down to slot";
        case OrchestratorState::InStor_GripperOpen_Cmd:
            return "Opening gripper";
        case OrchestratorState::InStor_GripperOpen_Wait:
            return "Waiting for gripper open";
        case OrchestratorState::InStor_RobotUpFromSlot_Cmd:
            return "Robot up from slot";
        case OrchestratorState::InStor_RobotUpFromSlot_Wait:
            return "Waiting for robot up from slot";
        case OrchestratorState::InStor_Complete:
            return "Input-to-storage complete";
        case OrchestratorState::Stopping:
            return "Stopping system";
        case OrchestratorState::Stopped:
            return "Stopped";
        case OrchestratorState::Faulted:
            return "Fault";
    }

    return "Unknown";
}
