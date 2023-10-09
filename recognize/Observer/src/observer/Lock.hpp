#pragma once

namespace Observer {
    template <typename T>
    concept BasicLockable = requires(T lockable) {
                                { lockable.lock() };
                                { lockable.unlock() };
                            };

    class NullLock final {
       public:
        void lock() {}
        void unlock() noexcept {}
    };
}  // namespace Observer