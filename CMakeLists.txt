cmake_minimum_required(VERSION 3.13)
project(cpprussia-workshop-condition-variable)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tasks: concurrency, condition_variable
set(TARGETS
      task-1
      task-2
      task-3
      task-4
      task-5
      task-6
      task-7
)

foreach(TARGET ${TARGETS})
  add_executable(${TARGET} src/${TARGET}/main.cpp)
endforeach()

# Dependencies setup
if(UNIX)
  foreach(TARGET ${TARGETS})
    target_link_libraries(${TARGET} PUBLIC pthread)
  endforeach()
endif()
