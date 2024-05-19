#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>
#include <thread>
#include <chrono>
#include <vector>
#include "tests.h"

// Требования к очереди:
// - first-in-first-out очередь
// - thread-safe
// - pop блокируется, если в очереди нет элементов, и разблокируется как только, как появляется хотя бы один элемент

template <typename T>
class ConcurrentFIFOQueue {
public:
    void push(const T& val) {
        // TODO
    }

    T pop() {
        // TODO
        return T{};
    }

private:
    std::mutex _m;
    std::condition_variable _not_empty_cv;
    std::queue<T> _queue;
};

/*
 * Тесты
 */
void test_multiple_push_pop() {
    ConcurrentFIFOQueue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.push(3);
    EXPECT(queue.pop() == 1);
    EXPECT(queue.pop() == 2);
    EXPECT(queue.pop() == 3);

    PASS();
}

void test_pop_wait() {
    ConcurrentFIFOQueue<int> queue;
    std::atomic<bool> item_popped{false};

    std::thread consumer{[&]() {
        queue.pop();
        item_popped.store(true);
    }};

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Подождали, проверяем, что pop все еще заблокирован
    EXPECT(item_popped.load() == false);

    // Разблокируем поток, ожидающий в pop
    queue.push(1);

    consumer.join();

    EXPECT(item_popped.load() == true);

    PASS();
}

void test_multiple_threads() {
    constexpr auto NumThreads = 4;
    constexpr auto N = 100;  // каждый producer поток производит N чисел (все уникальные)

    ConcurrentFIFOQueue<int> queue;

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

    // Проверяем, что из очереди было получено нужное кол-во объектов
    EXPECT(consumed.size() == N * NumThreads);

    std::sort(std::begin(consumed), std::end(consumed));

    for (int i = 1; i < N; ++i) {
        // Проверяем, что нет дублей и дырок
        EXPECT(consumed[i] == consumed[i - 1] + 1);
    }

    PASS();
}

int main() {
    try {
        test_multiple_push_pop();
        test_pop_wait();
        test_multiple_threads();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

/*
 * Усложнение:
 * - вопрос: в push должен быть notify_one или notify_all? в чем разница?
 *
 * - добавьте ConcurrentFIFOQueue::push(T &&) метод, который перемещает объект в контейнер, а не копирует
 *
 * - добавьте ConcurrentFIFOQueue::emplace метод, конструирующий объект in-place
 */
