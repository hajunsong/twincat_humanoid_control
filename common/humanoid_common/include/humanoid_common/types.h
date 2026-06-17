// humanoid_common/types.h
//
// Basic POD types shared between TwinCAT modules. These are cycle-safe:
// trivially constructible, no dynamic allocation, no exceptions.
//
// Pure C++. No TwinCAT / TcCOM / TMC / ADS / EtherCAT dependency.
#pragma once

#include <cstddef>
#include <cstdint>

namespace humanoid_common {

// Single-axis state sample. Layout intentionally simple so adapter code
// on the TwinCAT side may copy it via memcpy if required.
struct MotorStateSample {
    std::int64_t  timestamp_ns;
    double        position;
    double        velocity;
    double        torque;
    std::uint32_t status_flags;
    std::uint32_t error_code;
};

// Single-axis command sample.
struct MotorCommandSample {
    std::int64_t  timestamp_ns;
    double        target_position;
    double        target_velocity;
    double        target_torque;
    std::uint32_t control_mode;
    std::uint32_t reserved;
};

}  // namespace humanoid_common
