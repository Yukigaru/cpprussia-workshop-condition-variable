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

... Built target task_1
```

2) Запустите собранный бинарь. Убедитесь, что запускается.
```
> ./build/task_1

another thread: waiting...
main: signaling the other thread to resume
another thread: got the signal and resumed
```

3) Для сборки во время workshop'а вы можете напрямую вызывать компилятор в терминале или можете использовать свой IDE (но настраивать нужно будет самостоятельно).
