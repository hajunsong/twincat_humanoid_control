// humanoid_common/cia402.h
//
// CiA 402 (CANopen drive profile) status word, control word, and mode
// constants. Values are kept identical to the historical 31-motor
// reference module so behaviour ports cleanly.
//
// Pure C++. No TwinCAT / TcCOM / TMC / ADS / EtherCAT dependency.
#pragma once

#include <cstdint>

namespace humanoid {
namespace cia402 {

// Status word state-bit mask and individual flag(s)
constexpr std::uint16_t kStatusStateMask = 0x006FU;
constexpr std::uint16_t kStatusFaultBit  = 0x0008U;

// Control word commands
constexpr std::uint16_t kControlShutdown        = 0x0006U;
constexpr std::uint16_t kControlSwitchOn        = 0x0007U;
constexpr std::uint16_t kControlEnableOperation = 0x000FU;
constexpr std::uint16_t kControlFaultReset      = 0x0080U;

// Modes of operation (IEC 61800-7-200)
constexpr std::int16_t kModeCsp = 8;   // Cyclic synchronous position
constexpr std::int16_t kModeCsv = 9;   // Cyclic synchronous velocity
constexpr std::int16_t kModeCst = 10;  // Cyclic synchronous torque

// Extracts the state bits from a CiA 402 status word.
inline std::uint16_t StateBits(std::uint16_t status_word)
{
    return static_cast<std::uint16_t>(status_word & kStatusStateMask);
}

}  // namespace cia402
}  // namespace humanoid
