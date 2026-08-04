if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)

foreach(required
    "set(PROJECT_CURRENT_FDD_SMOKE_TARGETS"
    "set(PROJECT_CURRENT_FDD_HDD_SMOKE_TARGET vm-product-session-smoke)"
    "set(PROJECT_CURRENT_MEDIA_SMOKE_TARGETS"
    "foreach(target IN LISTS PROJECT_CURRENT_FDD_SMOKE_TARGETS)")
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Current media smoke classification is incomplete: ${required}")
    endif()
endforeach()

foreach(required
    [=[${PROJECT_CURRENT_FDD_SMOKE_TARGETS}]=]
    [=[${PROJECT_CURRENT_FDD_HDD_SMOKE_TARGET}]=]
    [=[project_add_current_smoke_test(${PROJECT_CURRENT_FDD_HDD_SMOKE_TARGET}]=])
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Current media smoke registration is not derived: ${required}")
    endif()
endforeach()

foreach(forbidden
    "foreach(target IN ITEMS vm-dos-prompt-smoke"
    "project_add_current_smoke_test(vm-product-session-smoke")
    string(FIND "${cmake_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Current media smoke registration is duplicated: ${forbidden}")
    endif()
endforeach()

message("M5:T204:S1:CURRENT-MEDIA-SMOKE-CLASSIFICATION:OK")
