#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include "tests.h"

using namespace std::chrono_literals;


class RWLock {
public:
    void lock_reader() {
        std::unique_lock l{_m};
        // ...
    }

    void unlock_reader() {
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
    l.lock_reader();
    l.unlock_reader();
    PASS();
}

void test_readers_dont_block() {
    RWLock l;
    l.lock_reader();
    std::thread t{[&]() {
        // не должно заблокироваться
        l.lock_reader();
        l.unlock_reader();
    }};
    t.join();
    l.unlock_reader();
    PASS();
}

void test_writer_blocks_reader() {
    RWLock l;
    l.lock();

    std::thread reader([&]() {
        auto start = std::chrono::steady_clock::now();
        l.lock_reader();
        l.unlock_reader();
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
    l.lock_reader();

    std::thread reader([&]() {
        auto start = std::chrono::steady_clock::now();
        l.lock();
        l.unlock();
        auto end = std::chrono::steady_clock::now();
        EXPECT(end - start >= 100ms);
    });

    std::this_thread::sleep_for(100ms);
    l.unlock_reader();

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

void test_high_contention() {
    RWLock l;

    constexpr auto NumThreads = 8;
    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < NumThreads; ++i) {
        threads.emplace_back([&](int idx) {
            while (std::chrono::steady_clock::now() - start < 100ms) {
                // половина потоков - читатели, половина - писатели
                if (idx % 2 == 0) {
                    l.lock_reader();
                    l.unlock_reader();
                } else {
                    // писатели дольше и реже держат lock
                    l.lock();
                    std::this_thread::sleep_for(10us);
                    l.unlock();
                    std::this_thread::sleep_for(20us);
                }
            }
        }, i);
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
        test_high_contention();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
