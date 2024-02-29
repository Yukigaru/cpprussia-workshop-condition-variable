#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>


template <typename T>
class ConcurrentFIFOQueue {
public:
    // note: добавлен лимит на размер очереди
    ConcurrentFIFOQueue(std::uint64_t limit): _limit(limit) {
    }

    void push(const T &val) {
        // ждёт, если очередь переполнена
    }

    T pop() {
        // ждёт, если нет элементов
        return T{};
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    std::deque<T> _queue;
    std::uint64_t _limit;
};


void worker_thread_func(ConcurrentFIFOQueue<int> &queue, unsigned idx) {
    while (true) {
        auto v = queue.pop();
        std::cout << "thread " << idx << ": got value " << v << std::endl;
    }
}

int main() {
    // очередь на 4 элемента
    ConcurrentFIFOQueue<int> q{4};

    std::vector<std::thread> threads;
    for (auto i = 0u; i < 8; i++) {
        threads.emplace_back(std::thread{worker_thread_func, std::ref(q), i});
    }

    // наполняем значениями
    for (auto i = 0u; i < 256; i++) {
        q.push(rand());
    }

    for (auto &t : threads) {
        t.join();
    }
    return 0;
}
