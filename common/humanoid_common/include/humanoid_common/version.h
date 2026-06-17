// humanoid_common/version.h
//
// Version constants for the humanoid_common header-only library.
//
// Pure C++. Does not include any TwinCAT, TcCOM, TMC, ADS, or EtherCAT
// headers. Only depends on freestanding C++ standard library headers.
#pragma once

#include <cstdint>

namespace humanoid_common {
namespace version {

constexpr std::uint32_t kMajor = 0;
constexpr std::uint32_t kMinor = 1;
constexpr std::uint32_t kPatch = 0;

// Header-only: defined inline so no separate .cpp / .lib is needed.
inline const char* GetVersionString() noexcept {
    return "humanoid_common 0.1.0";
}

}  // namespace version
}  // namespace humanoid_common
