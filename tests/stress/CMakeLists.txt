# The stress run: a program, not a unit test. Its scene drives the window through resizes and minimizes while
# drawing, and `--frames 600` makes it exit 1 if validation said anything. Linked like a game, through
# dilithium::main. Only StressScheduleTests.cpp here is swept into dilithium_tests.
add_executable(stress_run main.cpp StressScene.cpp)
target_link_libraries(stress_run PRIVATE dilithium::main)

dilithium_apply_guard_rails(stress_run)
