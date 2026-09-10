if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)
file(READ "${PROJECT_SOURCE_DIR}/src/lib/CMakeLists.txt" library_cmake_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/run_handle.c" runner_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/platform.c" coordinator_source)
file(READ "${PROJECT_SOURCE_DIR}/test/vm/machine/vm_platform_run_handle_contract_smoke.c"
    linux_smoke_source)

foreach(required
    "src/vm/platform/run_handle.c")
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux platform CMake contract is missing: ${required}")
    endif()
endforeach()

foreach(required
    "find_package(Curses REQUIRED)"
    "find_package(Threads REQUIRED)"
    "Threads::Threads")
    string(FIND "${library_cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux platform CMake contract is missing: ${required}")
    endif()
endforeach()

foreach(required
    "vm-platform-run-handle-contract-smoke"
    "test/vm/machine/vm_platform_run_handle_contract_smoke.c")
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux runtime probe is missing: ${required}")
    endif()
endforeach()

foreach(required
    "host_sync_task_join"
    "vm_platform_run_handle_finalize")
    string(FIND "${runner_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux run-handle contract is missing: ${required}")
    endif()
endforeach()

foreach(required
    "VM_PLATFORM_RUN_EVENT_STARTUP_FAILED"
    "vm_platform_run_context_set_display_mode")
    string(FIND "${coordinator_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux platform coordinator contract is missing: ${required}")
    endif()
endforeach()

string(FIND "${runner_source}" "VM_PLATFORM_RUN_EVENT_STOP_REQUESTED"
    keyboard_stop_position)
if(NOT keyboard_stop_position EQUAL -1)
    message(FATAL_ERROR "Linux keyboard platform must not own a lifecycle stop path")
endif()

string(REGEX MATCHALL "context->execution->stop\\(" direct_stop_calls
    "${runner_source}")
list(LENGTH direct_stop_calls direct_stop_count)
if(NOT direct_stop_count EQUAL 1)
    message(FATAL_ERROR
        "Linux platform must have exactly one execution stop request; found ${direct_stop_count}")
endif()

foreach(required
    "pthread_create"
    "VM_PLATFORM_RUN_EVENT_STOP_REQUESTED"
    "pthread_join"
    "!vm_platform_run_handle_is_active")
    string(FIND "${linux_smoke_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux run-handle smoke is incomplete: ${required}")
    endif()
endforeach()

message("M5:T196:S1:LINUX-PLATFORM-CONTRACT:OK")
