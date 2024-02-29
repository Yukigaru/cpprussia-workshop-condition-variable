#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>

// Требования к очереди:
// - thread-safe
// - fifo очередь
// - pop блокируется, если нет элементов

template <typename T>
class ConcurrentFIFOQueue {
public:
    ConcurrentFIFOQueue() {
    }

    void push(const T &val) {
        // TODO
    }

    T pop() {
        // TODO
        return T{};
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    std::deque<T> _queue;
};


void worker_thread_func(ConcurrentFIFOQueue<int> &queue, unsigned idx) {
    while (true) {
        auto v = queue.pop();
        std::cout << "thread " << idx << ": got value " << v << std::endl;
    }
}

int main() {
    ConcurrentFIFOQueue<int> q;

    std::vector<std::thread> threads;
    for (auto i = 0u; i < 8; i++) {
        threads.emplace_back(std::thread{worker_thread_func, std::ref(q), i});
    }

    // наполняем значениями
    for (auto i = 0u; i < 256; i++) {
        q.push(rand());
    }

    // завершение
    for (auto &t : threads) {
        t.join();
    }
    return 0;
}
