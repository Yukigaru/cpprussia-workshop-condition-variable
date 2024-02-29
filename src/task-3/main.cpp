#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>

// Пример data race: второй поток может не проснуться, несмотря на то, что был вызван notify_one и была изменена переменная.
// Нужно исправить ошибку.

std::atomic_bool resume;
std::condition_variable cv;
std::mutex m;

void another_thread_func() {
    std::cout << "another thread: waiting..." << std::endl;

    std::unique_lock l{m};
    cv.wait(l);

    std::cout << "another thread: got the signal and resumed" << std::endl;
}

int main() {
    std::thread t(another_thread_func);

    std::cout << "main: signaling the other thread to resume" << std::endl;

    cv.notify_one();

    t.join();
    return 0;
}
