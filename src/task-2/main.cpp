#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>

// Пример lost wakeup: второй поток вероятнее всего не проснётся, несмотря на то, что был вызван notify_one.
// Нужно исправить ошибку.

std::condition_variable cv;
std::mutex m;

void another_thread_func() {
    std::cout << "another: waiting..." << std::endl;

    std::unique_lock<std::mutex> l{m};
    cv.wait(l);

    std::cout << "another: got the signal and resumed" << std::endl;
}

void main_thread_func() {
    std::cout << "main: signaling the other thread to resume" << std::endl;

    cv.notify_one();
}

int main() {
    std::thread t1{another_thread_func};
    std::thread t2{main_thread_func};

    t1.join();
    t2.join();
    return 0;
}
