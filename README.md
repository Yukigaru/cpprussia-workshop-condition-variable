## Что будет предоставлено участникам
За сутки участникам будет предоставлена ссылка на Github репозиторий с задачами. Каждая задача в отдельном .cpp файле и компилируется в отдельный executable файл, для генерации сборки - CMake.

## План:

0) Введение и организационные моменты (10 минут).


1) Объяснение (5 мин): Зачем нужен `condition_variable`, какие проблемы/задачи решает.


2) Объяснение (5 мин): Показать пример использования CV и объяснить нюансы.


3) Задача 1 (10 мин + 5 мин на решение проблем): добавить `condition_variable` так, чтобы второй поток успешно получал resume сигнал и при не был в busy wait'е.
```
// Рабочий код, но нагружает cpu даже в ожидании.

std::atomic_bool resume;

void another_thread_func() {
    std::cout << "another thread: waiting..." << std::endl;

    while (!resume.load()) {
        // busy wait
    }

    std::cout << "another thread: got the signal and resumed" << std::endl;
}

int main() {
    std::thread t(another_thread_func);

    std::this_thread::sleep_for(3s);
    std::cout << "main: signaling the other thread to resume" << std::endl;

    // TODO: заменить на condition_variable
    resume.store(true);

    t.join();
    return 0;
}
```


4) Изучаем типовые ошибки с `condition_variable`. Исправляем написанные баги в заранее подготовленных программах.
Задача 2 (15 мин): О lost wakeups, и почему нельзя ждать без условия.
```
// Пример lost wakeup: второй поток вероятнее всего не проснётся, несмотря на то, что был вызван notify_one.
// Нужно исправить ошибку.

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>

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
```


Задача 3 (15 мин):
Изменили переменную, но не под mutex'ом.
```
// Пример data race: второй поток может не проснуться, несмотря на то, что был вызван notify_one и была изменена переменная.
// Нужно исправить ошибку.

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>

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
```


5) Задача 4 (25 мин): Напишем thread-safe queue с ожиданием элементов (на основе cond.var).
```
#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

// Требования к очереди:
// - thread-safe
// - fifo очередь
// - pop блокируется, если нет элементов

template <typename T>
class ConcurrentFIFOQueue {
public:
    ConcurrentFIFOQueue(std::uint64_t limit): _limit(limit) {
    }

    void push(const T &val) {
        // TODO
    }

    T pop() {
        // TODO
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    std::deque<T> _queue;
};


void worker_thread_func(ConcurrentFIFOQueue<int> &queue, unsigned idx) {
    while (true) {
        auto v = queue.pop();
        std::cout << "thread " << idx << " got value " << v << std::endl;
    };
}

int main() {
    ConcurrentFIFOQueue<int> q{5};
    std::vector<std::thread> threads;
    for (auto i = 0u; i < 8; i++) {
        threads.emplace_back(std::thread{worker_thread_func, queue, i});
    }

    // наполняем значениями
	for (auto i = 0u; i < 256; i++) {
        q.push(rand());
    }

    // завершение
    for (auto &t : threads) {
        t.join();
    }
    return 0;
}
```


6) Задача 6 (15 мин): Добавим в ConcurrentFIFOQueue ограничение на макс. размер очереди. Понадобится два `condition_variable`. Заготовка класса:
```
template <typename T>
class ConcurrentFIFOQueue {
public:
    ConcurrentFIFOQueue(std::uint64_t limit): _limit(limit) {
    }

    void push(const T &val) {
        // ждёт, если очередь переполнена
    }

    T pop() {
        // ждёт, если нет элементов
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    std::deque<T> _queue;
    std::uint64_t _limit;
};
```


7) Задача 7 (15 мин): Добавить в API очереди таймауты на ожидание:
```
class ConcurrentFIFOQueue {
public:
    void push(const T &val, std::chrono::steady_clock::duration timeout) {
        // ждёт, если очередь переполнена, но не дольше, чем timeout, если он задан
    }

    T pop(std::chrono::steady_clock::duration timeout) {
        // ждёт, если нет элементов, но не дольше, чем timeout, если он задан
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    std::deque<T> _queue;
    std::uint64_t _limit;
};
```

Итого: 1ч 50мин.

8) Дополнительная задача для быстрых: Добавим graceful shutdown по сигналу SIGINT. Если пользователь в терминале нажал Ctrl+C, то все потоки должны выйти из ожидания и корректно завершиться. Добавляем signal handler:
```
std::atomic_bool finish_program;

void handle_sigint(int) {
    queue.on_finish_signal();
}

class ConcurrentFIFOQueue {
public:
    void on_finish_signal() {
        // разблокировать все заблокированные потоки
    }
};
```
