add_executable(d02_triangle main.cpp TriangleScene.cpp)
target_link_libraries(d02_triangle PRIVATE dilithium::main)

dilithium_apply_guard_rails(d02_triangle)
