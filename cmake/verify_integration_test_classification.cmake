if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)

foreach(required
    "set(PROJECT_INTEGRATION_FDD_TARGETS"
    "set(PROJECT_INTEGRATION_FDD_HDD_TARGETS"
    "set(PROJECT_INTEGRATION_TEST_TARGETS"
    "foreach(target IN LISTS PROJECT_INTEGRATION_FDD_TARGETS)")
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Current media smoke classification is incomplete: ${required}")
    endif()
endforeach()

foreach(required
    [=[${PROJECT_INTEGRATION_FDD_TARGETS}]=]
    [=[${PROJECT_INTEGRATION_FDD_HDD_TARGETS}]=]
    [=[foreach(target IN LISTS PROJECT_INTEGRATION_FDD_HDD_TARGETS)]=])
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Current media smoke registration is not derived: ${required}")
    endif()
endforeach()

foreach(forbidden
    "foreach(target IN ITEMS vm-dos-prompt-smoke"
    "project_add_test(vm-product-session-smoke")
    string(FIND "${cmake_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Current media smoke registration is duplicated: ${forbidden}")
    endif()
endforeach()

message("M5:T204:S1:CURRENT-MEDIA-SMOKE-CLASSIFICATION:OK")
