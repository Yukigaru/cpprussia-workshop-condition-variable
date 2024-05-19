#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <stdexcept>
#include "tests.h"

// std::cv_status::timeout

template <typename T>
class ConcurrentFIFOQueue {
public:
    explicit ConcurrentFIFOQueue(size_t limit = 0) : _limit(limit) {}

    bool push(const T& val, std::chrono::steady_clock::duration timeout = std::chrono::seconds(0)) {
        std::unique_lock l{_m};
        // ждёт, если очередь переполнена, но не дольше, чем timeout (если он задан)
        // возвращает false, если случился timeout
        return true;
    }

    bool pop(T& out, std::chrono::steady_clock::duration timeout = std::chrono::seconds(0)) {
        std::unique_lock l{_m};
        // ждёт, если нет элементов, но не дольше, чем timeout (если он задан)
        // возвращает false, если случился timeout
        return true;
    }

private:
    std::mutex _m;

    std::queue<T> _queue;
    size_t _limit;
};

/*
 * Тесты
 */
void test_multiple_push_pop() {
    ConcurrentFIFOQueue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.push(3);

    int value;

    bool success = queue.pop(value);
    EXPECT(success);
    EXPECT(value == 1);

    success = queue.pop(value);
    EXPECT(success);
    EXPECT(value == 2);

    success = queue.pop(value);
    EXPECT(success);
    EXPECT(value == 3);

    PASS();
}

void test_pop_wait() {
    ConcurrentFIFOQueue<int> queue;
    std::atomic<bool> item_popped{false};

    std::thread consumer{[&]() {
        int unused;
        queue.pop(unused);
        item_popped.store(true);
    }};

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT(item_popped.load() == false);

    queue.push(1);

    consumer.join();

    EXPECT(item_popped.load() == true);

    PASS();
}

void test_push_wait() {
    constexpr auto Limit = 2u;
    ConcurrentFIFOQueue<int> queue{Limit};

    std::atomic_int values_pushed{0};

    std::thread producer([&]() {
        for (int i = 0; i < Limit + 1; ++i) {
            queue.push(i);
            values_pushed++;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT(values_pushed.load() == Limit);

    int unused;
    queue.pop(unused);
    producer.join();

    EXPECT(values_pushed.load() == Limit + 1);

    PASS();
}

void test_multiple_threads() {
    constexpr auto NumThreads = 4;
    constexpr auto N = 100;  // каждый producer поток производит N чисел

    ConcurrentFIFOQueue<int> queue{2};  // лимит в 2 элемента

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
            int num;
            queue.pop(num);

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

void test_push_with_timeout() {
    ConcurrentFIFOQueue<int> queue{1};

    // завершится без таймаута
    EXPECT(queue.push(1, std::chrono::seconds(1)));

    std::atomic<bool> timeout_occurred{false};

    std::thread producer([&]() {
        // случится таймаут
        if (!queue.push(2, std::chrono::milliseconds(100))) {
            timeout_occurred.store(true);
        }
    });

    EXPECT(!timeout_occurred.load());

    producer.join();

    EXPECT(timeout_occurred.load());

    PASS();
}

void test_pop_with_timeout() {
    ConcurrentFIFOQueue<int> queue;

    std::atomic<bool> timeout_occurred{false};
    int dummy;

    std::thread consumer([&]() {
        // случится таймаут
        if (!queue.pop(dummy, std::chrono::milliseconds(100))) {
            timeout_occurred.store(true);
        }
    });

    EXPECT(!timeout_occurred.load());

    consumer.join();

    EXPECT(timeout_occurred.load());

    PASS();
}

int main() {
    try {
        test_multiple_push_pop();
        test_pop_wait();
        test_push_wait();
        test_multiple_threads();

        test_push_with_timeout();
        test_pop_with_timeout();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
/* Усложнения:
 * - добавь метод stop_all(), который прерывает все ожидающие в push/pop потоки. По замыслу этот метод вызывается при завершении работы приложения. Добавь тесты.
 */
