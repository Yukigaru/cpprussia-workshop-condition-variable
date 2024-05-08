cmake_minimum_required(VERSION 3.13)
project(cpprussia-workshop-condition-variable)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_TSAN "Enable thread sanitizer" OFF)

# Exercises
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

  target_include_directories(${TARGET} PRIVATE src/common)
  if(ENABLE_TSAN)
    target_compile_options(${TARGET} PRIVATE -fsanitize=thread)
    target_link_options(${TARGET} PRIVATE -fsanitize=thread)
  endif()
endforeach()

# Dependencies setup
if(UNIX)
  foreach(TARGET ${TARGETS})
    target_link_libraries(${TARGET} PUBLIC pthread)
  endforeach()
endif()

# Testing
enable_testing()
foreach(TARGET ${TARGETS})
  add_test(NAME ${TARGET} COMMAND ${TARGET})
  set_tests_properties(${TARGET} PROPERTIES TIMEOUT 5) # 5 sec limit
endforeach()
