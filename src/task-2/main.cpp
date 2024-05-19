#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <vector>
#include <atomic>
#include "tests.h"

// ThreadFlag позволяет нескольким потокам ждать, пока другой поток не установит флаг на старт (set_flag).
// Флаг устанавливается один раз и навсегда. Если флаг уже установлен к моменту вызова wait(), тогда функция завершается
// сразу.
class ThreadFlag {
public:
    void wait() {
        // TODO
    }

    void set_flag() {
        // TODO
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    bool _flag{};
};

/*
 * Тесты
 */
void test_set_flag_before_wait() {
    ThreadFlag flag;
    flag.set_flag();  // Ставим флаг еще до ожидания

    std::thread test_thread([&]() {
        flag.wait();  // Не должно быть заблокировано
    });

    test_thread.join();

    PASS();
}

void test_wait_then_set_flag() {
    ThreadFlag flag;
    std::atomic_int waits_passed{0};

    static constexpr auto NumThreads = 8;

    // Стартуем несколько потоков, которые будут ждать флага
    std::vector<std::thread> test_threads;
    for (auto i = 0; i < NumThreads; i++) {
        test_threads.emplace_back(std::thread{[&]() {
            flag.wait();  // Заблокируется
            waits_passed++;
        }});
        test_threads.back().detach();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT(waits_passed == 0);  // Проверка, что потоки были и всё еще в ожидании

    flag.set_flag();  // Ставим флаг и разблокируем потоки

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT(waits_passed == NumThreads);  // Проверяем счетчик

    // Не завершается этот тест? Используешь ли ты notify_all, вместо notify_one?
    PASS();
}

void test_lost_wakeup() {
    for (auto i = 0u; i < 5000; i++) {
        ThreadFlag flag;

        std::thread t1([&]() {
            flag.wait();
        });
        std::thread t2([&]() {
            flag.set_flag();
        });

        t1.join();
        t2.join();
    }
    PASS();
}

int main() {
    try {
        test_set_flag_before_wait();
        test_wait_then_set_flag();
        test_lost_wakeup();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
