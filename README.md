## Что будет предоставлено участникам
За сутки участникам будет предоставлена ссылка на Github репозиторий с задачами.
Каждая задача в отдельном .cpp файле и компилируется в отдельный executable файл, для генерации сборки - CMake.
Для само-проверки в каждой задаче будут написаны unit-тесты и подключен thread sanitizer.
В конце workshop'а будут даны 1-2 задачи на последующую самостоятельную работу.

## План:

0) Введение и организационные моменты (10 минут).
- "О чем мы сегодня будем говорить и для кого (какой уровень)"
- Мы будем использовать только примитивы стандартной библиотеки
- Вопрос аудитории: есть ли проблемы со сборкой?
- Рассказать кратко о себе
- О том, что есть ассистирующий специалист и можно обращаться к нему

1) Объяснение (5 мин): Зачем нужен `condition_variable`, какие проблемы/задачи решает.


2) Объяснение (5 мин): Показать пример использования cond var и объяснить нюансы.


3) Задача 1 (10 мин + 5 мин на решение проблем): добавить `condition_variable` так, чтобы второй поток успешно получал resume сигнал и при не был в busy wait'е.

- Немного о том, как устроены задачи
- Про подключенный thread sanitizer

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

Опциональное усложнение: запустить N потоков вместо одного, и пробудить их все (`notify_all`).


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


Задача 3 (10 мин):
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

5) Задача 4 (10 мин): используем `notify_all` для построения примитива синхронизации ThreadFlag.
```
#include <condition_variable>
#include <iostream>
#include <thread>

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

        cv.notify_all();
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    bool _flag;
};
```


6) Задача 5 (20 мин): Напишем thread-safe queue с ожиданием элементов (на основе cond.var).
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
    ConcurrentFIFOQueue() {
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


7) Задача 6 (15 мин): Добавим в ConcurrentFIFOQueue ограничение на макс. размер очереди. Понадобится два `condition_variable`. Заготовка класса:
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


8) [Дополнительная задача] Добавим graceful shutdown по сигналу SIGINT. Если пользователь в терминале нажал Ctrl+C, то все потоки должны выйти из ожидания и корректно завершиться. Добавляем signal handler:
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


9) [Дополнительная задача] Реализовать свой вариант `mutex` и `condition_variable` на основе linux syscall'а `futex`.
Заготовка:
```
#include <chrono>
#include <thread>
#include <atomic>
#include <linux/futex.h>


class Mutex {
public:
    void lock() {
    }

    void unlock() {
    }
};


class ConditionVariable {
public:
    template <typename Predicate>
    void wait_for(Mutex& mutex, Predicate can_stop_waiting) {
    }

    template <class Rep, class Period, typename Predicate>
    void wait_until(Mutex& mutex, const std::chrono::duration<Rep, Period>& until_time, Predicate can_stop_waiting) {
    }

    void notify_one() {
    }

    void notify_all() {
    }

private:

};

```

10) [Дополнительная задача] Реализовать простой thread pool, использующий ConcurrentFIFOQueue для получения задач.
Заготовка:
```

class ThreadPool {
public:
    // Constructor: Initialize the thread pool and start threads
    ThreadPool(size_t numThreads) {
        // TODO: стартовать numThreads потоков, исполняющих workerFunction
    }

    // Destructor: Shut down the thread pool and join all threads
    ~ThreadPool() {
        // TODO: отправить потокам сигнал на завершение и подождать их
    }

    void submit(std::function<void()> task) {
    }

private:
    void workerFunction() {
    }

    std::vector<std::thread> workers;
    ConcurrentFIFOQueue<std::function<void()>> _task_queue;
};

```
