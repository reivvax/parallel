add_executable(parallel main.c monitor.c)
target_link_libraries(parallel io err atomic)
