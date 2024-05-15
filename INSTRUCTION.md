# Участникам

### До начала workshop'а

0) Установите необходимый toolchain, если ещё не установлен:
Ubuntu:
```
> sudo apt update && sudo apt install -y --no-install-recommends git cmake g++
```

1) Соберите проект. Убедитесь, что сборка проходит успешно.
```
> git submodule update --init --recursive
> mkdir build && cd build
> cmake ..
> cd ..
> cmake --build build

... Built target task-1
```

2) Запустите собранный бинарь. Убедитесь, что запускается.
```
> ./build/task-1

another thread: waiting...
main: signaling the other thread to resume
another thread: got the signal and resumed
```

3) Для сборки во время workshop'а вы можете напрямую вызывать компилятор в терминале, либо использовать свой IDE (но настраивать нужно будет самостоятельно).


### Troubleshooting
1) В случае ошибки при запуске бинарей:
```
FATAL: ThreadSanitizer: unexpected memory mapping 0x74dea7872000-0x74dea7d00000
```
Нужно выполнить:
```
sudo sysctl vm.mmap_rnd_bits=25
```
(работает до момента перезапуска)
