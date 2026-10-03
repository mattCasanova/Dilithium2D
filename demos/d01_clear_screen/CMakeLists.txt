add_executable(d01_clear_screen main.cpp)
target_link_libraries(d01_clear_screen PRIVATE dilithium::main)

dilithium_apply_guard_rails(d01_clear_screen)
