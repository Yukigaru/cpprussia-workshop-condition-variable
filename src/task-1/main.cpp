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
    std::cout << "another: waiting..." << std::endl;

    while (!resume.load()) {
        // TODO: замените busy-wait на засыпание с condition_variable
    }

    std::cout << "another: got the signal!" << std::endl;
}

void main_thread_func() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "main: signaling the other thread to resume" << std::endl;

    resume.store(true);
    // TODO: сделать condition_variable notify_one
}

int main() {
    std::thread t1{another_thread_func};
    std::thread t2{main_thread_func};

    t1.join();
    t2.join();
    return 0;
}
