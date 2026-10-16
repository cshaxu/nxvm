add_test(NAME x86-nxvm.cpu-bus-boundary-negative COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/cpu-bus-boundary-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/tests/cpu_bus_boundary_negative.cmake")
set_tests_properties(x86-nxvm.cpu-bus-boundary-negative PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 600)
add_test(NAME unit.fdc-boundary-negative COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fdc-boundary-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/tests/fdc_boundary_negative.cmake")
set_tests_properties(unit.fdc-boundary-negative PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
add_test(NAME x86-nxvm.test-route-partition COMMAND "${CMAKE_COMMAND}"
    "-DTEST_DIRECTORY=${CMAKE_BINARY_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/tests/verify_test_route_partition.cmake")
add_test(NAME x86-nxvm.test-route-partition-negative COMMAND "${CMAKE_COMMAND}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/route-partition-probe"
    -P "${CMAKE_CURRENT_LIST_DIR}/tests/verify_test_route_partition_negative.cmake")
add_test(NAME x86-nxvm.app-test-support-boundary COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/tests/verify_app_test_support_boundary.cmake")
if(POWERSHELL_EXECUTABLE)
    add_test(NAME x86-nxvm.dependency-dag-negative COMMAND "${CMAKE_COMMAND}"
        "-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
        "-DPOWERSHELL_EXECUTABLE=${POWERSHELL_EXECUTABLE}"
        "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/dependency-dag-negative"
        -P "${CMAKE_CURRENT_LIST_DIR}/tests/dependency_dag_negative.cmake")
endif()
set_tests_properties(x86-nxvm.test-route-partition
    x86-nxvm.test-route-partition-negative x86-nxvm.app-test-support-boundary
    PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
if(POWERSHELL_EXECUTABLE)
    set_tests_properties(x86-nxvm.dependency-dag-negative
        PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
endif()
