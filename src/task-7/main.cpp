#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>
#include <stdexcept>


const auto NumThreads = 8u;
const auto NumValues = 256u;


struct TimeoutException: public std::exception {
};


template <typename T>
class ConcurrentFIFOQueue {
public:
    void push(const T &val, std::chrono::steady_clock::duration timeout) {
        // ждёт, если очередь переполнена, но не дольше, чем timeout, если он задан
        // бросает исключение TimeoutException, если случился timeout
    }

    T pop(std::chrono::steady_clock::duration timeout) {
        // ждёт, если нет элементов, но не дольше, чем timeout, если он задан
        // бросает исключение TimeoutException, если случился timeout
        return T{};
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    std::deque<T> _queue;
    std::uint64_t _limit;
};


void worker_thread_func(ConcurrentFIFOQueue<int> &queue, unsigned idx) {
    // каждый поток пытается взять больше значений, чем будет в очереди
    try {
        for (int i = 0; i < 500; i++) {
            auto v = queue.pop(std::chrono::seconds(1));

            std::cout << "thread " << idx << ": got value " << v << std::endl;
        }

    } catch (TimeoutException) {
        std::cout << "thread " << idx << ": timeout, will finish" << std::endl;
    }
}

int main() {
    ConcurrentFIFOQueue<int> q;

    std::vector<std::thread> threads;
    for (auto i = 0u; i < NumThreads; i++) {
        threads.emplace_back(std::thread{worker_thread_func, std::ref(q), i});
    }

    for (auto i = 0u; i < NumValues; i++) {
        q.push(rand(), std::chrono::seconds(1));
    }

    for (auto &t : threads) {
        t.join();
    }
    return 0;
}
