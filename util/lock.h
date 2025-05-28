#ifndef LOCK_H
#define LOCK_H

#include <atomic>
#include <iostream>
#include <mutex>

namespace aria {
class Lock
{
public:
    Lock()
        : count{0}
    {}

    void lock()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++count;
    }

    bool unlock()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (count < 0) {
            std::cerr << "Error: Unlock called more times than lock. Current count: "
                      << count.load() << std::endl;
            return false;
        }
        --count;
        return true;
    }

    bool available() const { return count == 0; }

private:
    std::atomic<int> count;
    std::mutex mutex_;
};
} // namespace aria

#endif // LOCK_H
