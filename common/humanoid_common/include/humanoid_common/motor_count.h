// humanoid_common/motor_count.h
//
// Motor count constants. The library itself does not bake these counts
// into any algorithm; they are exposed so each TwinCAT module can
// instantiate templates with the correct N.
//
// Pure C++. No TwinCAT / TcCOM / TMC / ADS / EtherCAT dependency.
#pragma once

#include <cstddef>

namespace humanoid_common {
namespace motor_count {

constexpr std::size_t kUpperBodyMotors = 16;
constexpr std::size_t kLowerBodyMotors = 15;

}  // namespace motor_count
}  // namespace humanoid_common
