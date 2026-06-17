// humanoid_common/motor_array.h
//
// Motor-count-parameterised storage templates. Algorithms in this header
// must NOT hard-code upper-body or lower-body motor counts; the count is
// always the template parameter N.
//
// Real-time / cyclic safety:
//   - No dynamic allocation, no exceptions, no malloc/free/new/delete.
//   - No blocking I/O, no mutex acquisition.
//   - No standard-library facilities that may allocate.
//
// Pure C++. No TwinCAT / TcCOM / TMC / ADS / EtherCAT dependency.
#pragma once

#include <cstddef>

#include "humanoid_common/types.h"

namespace humanoid_common {

// Fixed-size array of motor-state samples.
template <std::size_t N>
struct MotorStateArray {
    static_assert(N > 0, "MotorStateArray requires at least one motor");

    static constexpr std::size_t kCount = N;

    MotorStateSample data[N];

    constexpr std::size_t size() const noexcept { return N; }

    void Reset() noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] = MotorStateSample{};
        }
    }
};

// Fixed-size array of motor-command samples.
template <std::size_t N>
struct MotorCommandArray {
    static_assert(N > 0, "MotorCommandArray requires at least one motor");

    static constexpr std::size_t kCount = N;

    MotorCommandSample data[N];

    constexpr std::size_t size() const noexcept { return N; }

    void Reset() noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] = MotorCommandSample{};
        }
    }
};

}  // namespace humanoid_common
