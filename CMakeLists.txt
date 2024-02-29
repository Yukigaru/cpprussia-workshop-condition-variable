cmake_minimum_required(VERSION 3.13)
project(cpprussia-workshop-condition-variable)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Test framework
add_subdirectory(thirdparty/googletest)

# Tasks: concurrency, condition_variable
set(TARGETS
      task_1
      task_2
      task_3
)

foreach(TARGET ${TARGETS})
  add_executable(${TARGET} src/${TARGET}/main.cpp)
endforeach()

# Dependencies setup
foreach(TARGET ${TARGETS})
  target_link_libraries(${TARGET} PUBLIC gtest gmock)
endforeach()

if(UNIX)
  foreach(TARGET ${TARGETS})
    target_link_libraries(${TARGET} PUBLIC pthread)
  endforeach()
endif()
