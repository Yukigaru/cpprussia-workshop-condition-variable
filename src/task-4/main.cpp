#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <vector>


// ThreadFlag позволяет N потокам ждать, пока другой поток не установит флаг на старт (set_flag).

class ThreadFlag {
public:
    ThreadFlag(): _flag(false) {
    }

    void wait() {
        // TODO
    }

    void set_flag() {
        // TODO

        _cv.notify_all();
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    bool _flag;
};


int main() {
    const size_t numThreads = 5;
    ThreadFlag startFlag;

    // Function that each thread will run
    auto threadFunction = [&startFlag]() {
        std::cout << "thread " << std::this_thread::get_id() << ": is waiting for the signal." << std::endl;

        startFlag.wait();

        std::cout << "thread " << std::this_thread::get_id() << ": has started imitating some work." << std::endl;
    };

    // Create and start threads
    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadFunction);
    }

    // Simulate some work or delay in the main thread
    std::this_thread::sleep_for(std::chrono::seconds(2));  // Main thread does other work

    // Signal all threads to start
    std::cout << "main: signaling all threads to start" << std::endl;
    startFlag.set_flag();

    // Join all threads with the main thread
    for (auto &th : threads) {
        th.join();
    }

    return 0;
}
