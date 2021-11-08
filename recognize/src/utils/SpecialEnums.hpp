#pragma once

// Consider the use of: https://github.com/Neargye/magic_enum

namespace Observer {
    template <typename TF, typename TB>
    inline bool has_flag(const TF flag, const TB bit) noexcept {
        return (flag & bit) == bit;
    }

    template <typename TL, typename TF>
    inline void set_flag(TL& lhs, const TF flag) noexcept {
        lhs |= flag;
    }

    template <typename TL, typename TF>
    inline void clear_flag(TL& lhs, const TF flag) noexcept {
        lhs &= ~flag;
    }
    
    template <typename TF>
    inline int flag_to_int(const TF flag) noexcept {
        return static_cast<int>(flag);
    }
}