#pragma once
#include <stdint.h>

namespace TMC2209Reg {
    constexpr uint8_t GCONF        = 0x00;
    constexpr uint8_t GSTAT        = 0x01;
    constexpr uint8_t IFCNT        = 0x02;
    constexpr uint8_t SLAVECONF    = 0x03;
    constexpr uint8_t OTP_PROG     = 0x04;
    constexpr uint8_t OTP_READ     = 0x05;
    constexpr uint8_t IOIN         = 0x06;
    constexpr uint8_t FACTORY_CONF = 0x07;

    constexpr uint8_t IHOLD_IRUN   = 0x10;
    constexpr uint8_t TPOWERDOWN   = 0x11;
    constexpr uint8_t TSTEP        = 0x12;
    constexpr uint8_t TPWMTHRS     = 0x13;
    constexpr uint8_t TCOOLTHRS    = 0x14;
    constexpr uint8_t VACTUAL      = 0x22;

    constexpr uint8_t SGTHRS       = 0x40;
    constexpr uint8_t SG_RESULT    = 0x41;
    constexpr uint8_t COOLCONF     = 0x42;

    constexpr uint8_t MSCNT        = 0x6A;
    constexpr uint8_t MSCURACT     = 0x6B;
    constexpr uint8_t CHOPCONF     = 0x6C;
    constexpr uint8_t DRV_STATUS   = 0x6F;
    constexpr uint8_t PWMCONF      = 0x70;
    constexpr uint8_t PWM_SCALE    = 0x71;
    constexpr uint8_t PWM_AUTO     = 0x72;
}

namespace TMC2209Bits {
    namespace GCONF {
        constexpr uint32_t I_SCALE_ANALOG   = 1u << 0;
        constexpr uint32_t INTERNAL_RSENSE  = 1u << 1;
        constexpr uint32_t EN_SPREADCYCLE   = 1u << 2;
        constexpr uint32_t SHAFT            = 1u << 3;
        constexpr uint32_t INDEX_OTPW       = 1u << 4;
        constexpr uint32_t INDEX_STEP       = 1u << 5;
        constexpr uint32_t PDN_DISABLE      = 1u << 6;
        constexpr uint32_t MSTEP_REG_SELECT = 1u << 7;
        constexpr uint32_t MULTISTEP_FILT   = 1u << 8;
    }

    namespace CHOPCONF {
        constexpr uint32_t TOFF_SHIFT   = 0;
        constexpr uint32_t HSTRT_SHIFT  = 4;
        constexpr uint32_t HEND_SHIFT   = 7;
        constexpr uint32_t TBL_SHIFT    = 15;
        constexpr uint32_t VSENSE       = 1u << 17;
        constexpr uint32_t MRES_SHIFT   = 24;
        constexpr uint32_t INTPOL       = 1u << 28;
        constexpr uint32_t DEDGE        = 1u << 29;
        constexpr uint32_t DISS2G       = 1u << 30;
        constexpr uint32_t DISS2VS      = 1u << 31;
    }

    namespace IOIN {
        constexpr uint32_t DIAG = 1u << 4;
    }

    namespace DRV_STATUS {
        constexpr uint32_t STEALTH = 1u << 30;
        constexpr uint32_t STST    = 1u << 31;
    }
}
