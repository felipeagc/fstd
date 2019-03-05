cmake_minimum_required(VERSION 3.10)
project("fstd")

set(
  FSTD_SOURCES
  fstd/general_allocator.c
  fstd/task_scheduler.c
  fstd/thread.c
  )

find_package(Threads)

add_library(fstd SHARED ${FSTD_SOURCES})
target_link_libraries(fstd PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(fstd PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(fstd PRIVATE "-std=c99")

option(FSTD_BUILD_TESTS "Build the tests" ON)
if(FSTD_BUILD_TESTS)
  add_subdirectory(tests)
endif()
