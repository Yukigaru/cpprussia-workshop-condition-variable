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

void waiting_thread_func() {
    std::cout << "another: waiting..." << std::endl;

    // TODO: замените busy-wait на засыпание с condition_variable
    while (!resume.load()) {
    }

    std::cout << "another: got the signal!" << std::endl;
}

int main() {
    std::thread t{waiting_thread_func};

    std::cout << "main: waiting 3 sec..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "main: signaling the other thread to resume" << std::endl;
    resume.store(true);

    t.join();
    return 0;
}
