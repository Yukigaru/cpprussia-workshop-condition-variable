#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <vector>
#include <atomic>
#include "tests.h"

// Latch - примитив синхронизации, позволяющий набору потоков подождать друг друга и стартовать вместе.
// Защёлка инициализируется со счётчиком, равным количеству ожидаемых потоков.
// Потоки блокируются в arrive_and_wait, уменьшая при этом счётчик, пока он не обнулится,
// после чего все потоки разблокируются.
class Latch {
  public:
    Latch(int64_t threads_expected): _counter(threads_expected) {
    }

    void arrive_and_wait() {
      std::unique_lock l{m};
      // ...
    }
  private:
    std::condition_variable cv;
    std::mutex m;
    int64_t _counter;
};

/*
 * Тесты
 */
void test_latch_synchronizes_threads() {
    constexpr auto num_threads = 16;

    Latch latch{num_threads};
    std::atomic<int> counter{0};

    auto worker = [&]() {
        counter.fetch_add(1);
        latch.arrive_and_wait();
        EXPECT(counter.load(std::memory_order_relaxed) == num_threads);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    PASS();
}

void test_latch_doesnt_reset() {
    Latch latch{2};
    bool passed = false;

    auto func = [&]() {
        latch.arrive_and_wait();
        passed = true;
    };
    std::thread t1{func};
    std::thread t2{func};
    t1.join();
    t2.join();

    // не должно заблокироваться
    std::thread t3{[&]() {
        latch.arrive_and_wait();
    }};
    t3.join();
    PASS();
}

int main() {
    try {
      test_latch_synchronizes_threads();
      test_latch_doesnt_reset();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
/*
 * Усложнение:
 * - реализовать многоразовый Barrier с тем же методом arrive_and_wait
 * */
