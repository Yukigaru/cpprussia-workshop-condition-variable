#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include "tests.h"

using namespace std::chrono_literals;

class RWLock {
public:
    void lock_shared() {
        std::unique_lock l{_m};
        // ...
    }

    void unlock_shared() {
        std::unique_lock l{_m};
        // ...
    }

    void lock() {
        std::unique_lock l{_m};
        // ...
    }

    void unlock() {
        std::unique_lock l{_m};
        // ...
    }

private:
    std::mutex _m;
    int _readers_count{0};
    int _writers_count{0};
};

/*
 * Тесты
 */
void test_simple() {
    RWLock l;
    l.lock();
    l.unlock();
    l.lock_shared();
    l.unlock_shared();
    PASS();
}

void test_readers_dont_block() {
    RWLock l;
    l.lock_shared();
    std::thread t{[&]() {
        // Не должно заблокироваться
        l.lock_shared();
        l.unlock_shared();
    }};
    t.join();
    l.unlock_shared();
    // Если дошли, значит читатели не блокируют друг друга
    PASS();
}

void test_writer_blocks_reader() {
    RWLock l;
    l.lock();

    std::thread reader([&]() {
        auto start = std::chrono::steady_clock::now();
        l.lock_shared();
        l.unlock_shared();
        auto end = std::chrono::steady_clock::now();
        EXPECT(end - start >= 100ms);
    });

    std::this_thread::sleep_for(100ms);
    l.unlock();

    reader.join();
    PASS();
}

void test_reader_blocks_writer() {
    RWLock l;
    l.lock_shared();

    std::thread reader([&]() {
        auto start = std::chrono::steady_clock::now();
        l.lock();
        l.unlock();
        auto end = std::chrono::steady_clock::now();
        EXPECT(end - start >= 100ms);
    });

    std::this_thread::sleep_for(100ms);
    l.unlock_shared();

    reader.join();
    PASS();
}

void test_two_writers_block_each_other() {
    RWLock l;
    l.lock();

    std::thread writer2([&]() {
        auto start = std::chrono::steady_clock::now();
        l.lock();
        l.unlock();
        auto end = std::chrono::steady_clock::now();
        EXPECT(end - start >= 100ms);
    });

    std::this_thread::sleep_for(100ms);
    l.unlock();

    writer2.join();
    PASS();
}

void test_many_threads() {
    constexpr auto NumThreads = 8;
    RWLock l;

    auto start = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < NumThreads; ++i) {
        threads.emplace_back(
            [&](int idx) {
                while (std::chrono::steady_clock::now() - start < 100ms) {
                    // Половина потоков - читатели, половина - писатели
                    if (idx % 2 == 0) {
                        l.lock_shared();
                        l.unlock_shared();
                    } else {
                        // Писатели дольше и реже держат lock
                        l.lock();
                        std::this_thread::sleep_for(10us);
                        l.unlock();
                        std::this_thread::sleep_for(20us);
                    }
                }
            },
            i);
    }

    for (auto& t : threads) {
        t.join();
    }
    PASS();
}

int main() {
    try {
        test_simple();
        test_readers_dont_block();
        test_writer_blocks_reader();
        test_reader_blocks_writer();
        test_two_writers_block_each_other();
        test_many_threads();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
/* Усложнение:
 * - добавь метод `bool try_lock()`, сразу (без блокировки) возвращающий true только при успешном взятии мьютекса.
 *
 * - добавь метод `try_lock_for(duration)`, который блокируется в попытке взять мьютекс не дольше duration, и возвращает true в случае успешного взятия мьютеса.
 */
