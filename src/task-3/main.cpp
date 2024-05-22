#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <vector>
#include <atomic>
#include "tests.h"

using namespace std::chrono_literals;

// Latch - примитив синхронизации, позволяющий набору потоков подождать друг друга и стартовать вместе.
// Защёлка инициализируется со счётчиком, равным количеству ожидаемых потоков.
// Потоки блокируются в arrive_and_wait, уменьшая при этом счётчик, пока он не обнулится,
// после чего все потоки разблокируются.
class Latch {
public:
    explicit Latch(int64_t threads_expected) {}

    void arrive_and_wait() {
        std::unique_lock l{_m};
        // ...
    }

private:
    std::condition_variable _cv;
    std::mutex _m;
    int64_t _counter;
};

/*
 * Тесты
 */
void test_latch_synchronizes_threads() {
    constexpr auto num_threads = 16;

    Latch latch{num_threads};

    auto worker = [&]() { latch.arrive_and_wait(); };

    std::vector<std::thread> threads;
    for (auto i = 0u; i < num_threads; i++) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }
    // Если дошли до PASS, значит все потоки завершились
    PASS();
}

void test_latch_awaits() {
    Latch latch{3};

    auto worker = [&]() {
        auto start = std::chrono::steady_clock::now();
        latch.arrive_and_wait();

        // Проверяем, что потоки были в ожидании не менее 100 ms
        auto end = std::chrono::steady_clock::now();
        EXPECT(end - start >= 100ms);
    };

    std::thread t1{worker};
    std::thread t2{worker};

    // Третий поток задерживает остальных на 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    latch.arrive_and_wait();

    t1.join();
    t2.join();
    PASS();
}

void test_latch_doesnt_reset() {
    Latch latch{2};

    auto func = [&]() { latch.arrive_and_wait(); };
    std::thread t1{func};
    std::thread t2{func};
    t1.join();
    t2.join();

    // Не должно заблокироваться, т.к. один раз latch уже сработал
    std::thread t3{[&]() { latch.arrive_and_wait(); }};
    t3.join();
    PASS();
}

int main() {
    try {
        test_latch_synchronizes_threads();
        test_latch_awaits();
        test_latch_doesnt_reset();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
/*
 * Усложнения:
 * - добавить метод `wait`, блокирующий, пока counter не станет равен 0, но не меняющий сам счётчик.
 *
 * - реализовать многоразовый Barrier с тем же методом arrive_and_wait
 * */
