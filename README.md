# C++ Russia 2024, workshop "C++ concurrency: condition variables"

### Что такое
Это репозиторий к практическому воркшопу, во время которого вы изучите безопасную синхронизацию между потоками на основе condition variable.
В планах:
- узнаете, что такое `std::condition_variable` и какие задачи он решает;
- изучите типовые ошибки при использовании и к чему они приводят;
- вживую поймаете spurious wakeup;
- напишете несколько concurrency-примитивов на основе condition variable.

Воркшоп разделен на 7 задач по 5-30 минут. Перед началом каждой задачи будет теоретический материал, а после - разбор типового решения.

Для быстрых и опытных учеников в конце .cpp файла будет «звездочка»: опциональное усложнение, которое можно дополнительно реализовать.


### Требования

ОС: Любой Linux дистрибутив, Windows с MSVC или WSL.
Компилятор: GCC 8.x, Clang 5.x, с поддержкой C++17.
На остальных платформах работа возможна, но не проверена.


### До начала

1) Установите необходимый toolchain, если ещё не установлен: `make`, `git`, `cmake`, `g++` (либо `clang`, либо `visual studio` на Windows).

2) Убедитесь, что сборка проходит успешно.
```
> cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug .
> cmake --build build
```

3) Для сборки во время workshop'а вы можете использовать cmake в терминале, либо использовать свой IDE (настраивать нужно будет самостоятельно).


### Troubleshooting
1) В случае ошибки при запуске бинарей:
```
FATAL: ThreadSanitizer: unexpected memory mapping 0x74dea7872000-0x74dea7d00000
```
Нужно выполнить:
```
sudo sysctl vm.mmap_rnd_bits=28
```
(работает до момента перезапуска)
