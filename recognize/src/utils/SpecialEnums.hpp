#pragma once

// Consider the use of: https://github.com/Neargye/magic_enum

namespace Observer {
    template <typename TF, typename TB>
    inline bool is_bit_flag(const TF flag, const TB bit) noexcept {
        return (flag & bit) == bit;
    }
    
    template <typename TF>
    inline int flag_to_int(const TF flag) noexcept {
        return static_cast<int>(flag);
    }
}