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


4) Изучаем типовые ошибки с `condition_variable`. Исправляем написанные баги в заранее подготовленных программах.
Задача 2 (15 мин): О lost wakeups, и почему нельзя ждать без условия.

Задача 3 (10 мин):
Изменили переменную, но не под mutex'ом.


5) Задача 4 (10 мин): используем `notify_all` для построения примитива синхронизации ThreadFlag.


6) Задача 5 (20 мин): Напишем thread-safe queue с ожиданием элементов (на основе cond.var).


7) Задача 6 (15 мин): Добавим в ConcurrentFIFOQueue ограничение на макс. размер очереди. Понадобится два `condition_variable`. Заготовка класса:


8) Задача 7 (15 мин): Добавить в API очереди таймауты на ожидание:


9) [Дополнительная задача] Добавим graceful shutdown по сигналу SIGINT. Если пользователь в терминале нажал Ctrl+C, то все потоки должны выйти из ожидания и корректно завершиться. Добавляем signal handler:


10) [Дополнительная задача] Реализовать свой вариант `mutex` и `condition_variable` на основе linux syscall'а `futex`.
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

11) [Дополнительная задача] Реализовать простой thread pool, использующий ConcurrentFIFOQueue для получения задач.
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
