# Explicit benchmark only: timing variance is not an ordinary CTest assertion.
add_executable(mynes-performance-probe performance_probe.c)
target_link_libraries(mynes-performance-probe PRIVATE mynes-machine-driver base storage)
mynes_enable_strict_warnings(mynes-performance-probe)

# Bounded owner-ROM observations, not an automatic CTest assertion.
add_executable(mynes-controller-input-probe EXCLUDE_FROM_ALL controller_input_probe.c)
target_link_libraries(mynes-controller-input-probe PRIVATE mynes-machine-driver kvm-base)
mynes_enable_strict_warnings(mynes-controller-input-probe)
