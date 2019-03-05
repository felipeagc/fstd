add_subdirectory(unity)

add_executable(general_allocator_tests general_allocator_tests.c)
target_link_libraries(general_allocator_tests PRIVATE unity fstd)
target_compile_options(general_allocator_tests PRIVATE "-std=c99")
