#include <atomic>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

/*
Рабочий код, но нагружает CPU даже в ожидании.
Необходимо переписать с использованием condition_variable.
*/

std::atomic_bool resume{false};

void another_thread_func() {
    std::cout << "another thread: waiting..." << std::endl;

    while (!resume.load()) {
        // busy wait
    }

    std::cout << "another thread: got the signal and resumed" << std::endl;
}

int main() {
    std::thread t{another_thread_func};

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "main: signaling the other thread to resume" << std::endl;

    resume.store(true);
    // TODO: notify a condition_variable

    t.join();
    return 0;
}
