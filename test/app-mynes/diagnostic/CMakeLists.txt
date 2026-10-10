# Explicit benchmark only: timing variance is not an ordinary CTest assertion.
add_executable(mynes-performance-probe performance_probe.c)
target_link_libraries(mynes-performance-probe PRIVATE mynes-core-driver base storage)
mynes_enable_strict_warnings(mynes-performance-probe)
