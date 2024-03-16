#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>

// Пример race condition: переменная resume меняется не под мьютексом.
// Нужно исправить ошибку.

bool resume{false};
std::condition_variable cv;
std::mutex m;

void first_thread_func() {
    std::cout << "thread 1: waiting...\n";

    std::unique_lock<std::mutex> l{m};

    while (!resume) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        cv.wait(l);
    }

    std::cout << "thread 1: got the signal and resumed\n";
}

void second_thread_func() {
    std::cout << "thread 2: signaling the other thread to resume\n";

    resume = true;
    cv.notify_one();
}

int main() {
    std::thread t1(first_thread_func);
    std::thread t2(second_thread_func);

    t1.join();
    t2.join();
    return 0;
}
/*
 * Усложнение:
 * - почему использование std::atomic_bool вместо bool не поможет?
 * - изучите и попробуйте std::condition_variable_any вместе с std::shared_lock
 * */
