#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <deque>
#include <algorithm>
#include <cstring>  // strerror
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <limits.h>
#include "tests.h"

// Futex system call обёртки
int futex_wait(void* addr, int val_expected) {
    // See: https://man7.org/linux/man-pages/man2/futex.2.html
    return syscall(SYS_futex, addr, FUTEX_WAIT, val_expected, NULL, NULL, 0);
}

int futex_wake(void* addr, int nwakeups) {
    return syscall(SYS_futex, addr, FUTEX_WAKE, nwakeups, NULL, NULL, 0);
}

class ConditionVariable {
public:
    ConditionVariable() {}

    template <typename Predicate>
    void wait(std::unique_lock<std::mutex>& lock, Predicate pred) {
        // ...
    }

    void notify_one() {
        // ...
    }

private:
    std::atomic_int _wake;
};

/*
 * Тесты
 */
void test_single_thread_notify() {
    std::mutex mtx;
    ConditionVariable cv;
    bool ready{false};

    std::thread t([&]() {
        std::unique_lock l{mtx};
        cv.wait(l, [&]() { return ready; });
    });

    {
        std::unique_lock l{mtx};
        ready = true;
        cv.notify_one();
    }
    t.join();
    PASS();
}

template <typename T>
class Queue {
public:
    Queue(size_t limit) : _limit(limit) {}

    void push(const T& val) {
        std::unique_lock l{_m};
        _not_full_cv.wait(l, [&]() { return _queue.size() < _limit; });
        _queue.push_front(val);
        _not_empty_cv.notify_one();
    }

    T pop() {
        std::unique_lock l{_m};
        _not_empty_cv.wait(l, [&]() { return !_queue.empty(); });
        auto val = _queue.back();
        _queue.pop_back();
        _not_full_cv.notify_one();
        return val;
    }

private:
    std::mutex _m;
    ConditionVariable _not_empty_cv;
    ConditionVariable _not_full_cv;
    std::deque<T> _queue;
    size_t _limit;
};

void test_concurrent_queue() {
    constexpr auto NumThreads = 4;
    constexpr auto N = 1000;  // каждый producer поток производит N чисел

    Queue<int> queue{2};

    std::vector<int> consumed;
    std::mutex consumed_mutex;

    auto producer_func = [&](int thread_num) {
        for (int i = 0; i < N; ++i) {
            int num = thread_num * N + i;
            queue.push(num);
        }
    };

    auto consumer_func = [&]() {
        for (int i = 0; i < N; ++i) {
            int num = queue.pop();

            std::lock_guard<std::mutex> lock(consumed_mutex);
            consumed.push_back(num);
        }
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < NumThreads; ++i) {
        threads.emplace_back(producer_func, i);
        threads.emplace_back(consumer_func);
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT(consumed.size() == N * NumThreads);

    std::sort(std::begin(consumed), std::end(consumed));

    for (int i = 1; i < N; ++i) {
        EXPECT(consumed[i] == consumed[i - 1] + 1);
    }

    PASS();
}

int main() {
    try {
        test_single_thread_notify();
        test_concurrent_queue();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
/*
 * Усложнение:
 * - реализовать notify_all
 * - реализовать wait с таймаутом
 */
