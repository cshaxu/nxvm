if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/linux/linuxcon.c" linuxcon_source)
file(READ "${PROJECT_SOURCE_DIR}/test/vm/machine/vm_platform_linux_run_handle_smoke.c"
    linux_smoke_source)

foreach(required
    "src/vm/platform/linux/linux.c"
    "src/vm/platform/linux/linuxcon.c"
    "find_package(Curses REQUIRED)"
    "find_package(Threads REQUIRED)"
    "Threads::Threads")
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux platform CMake contract is missing: ${required}")
    endif()
endforeach()

foreach(required
    "vm-platform-linux-run-handle-smoke"
    "test/vm/machine/vm_platform_linux_run_handle_smoke.c")
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux runtime probe is missing: ${required}")
    endif()
endforeach()

foreach(required
    "VM_PLATFORM_RUN_EVENT_STARTUP_FAILED"
    "pthread_join"
    "vm_platform_linuxcon_run_handle_finalize")
    string(FIND "${linuxcon_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Linux run-handle contract is missing: ${required}")
    endif()
endforeach()

string(FIND "${linuxcon_source}" "VM_PLATFORM_RUN_EVENT_STOP_REQUESTED"
    keyboard_stop_position)
if(NOT keyboard_stop_position EQUAL -1)
    message(FATAL_ERROR "Linux keyboard platform must not own a lifecycle stop path")
endif()

string(REGEX MATCHALL "vm_platform_execution_stop_for" direct_stop_calls
    "${linuxcon_source}")
list(LENGTH direct_stop_calls direct_stop_count)
if(NOT direct_stop_count EQUAL 1)
    message(FATAL_ERROR
        "Linux platform must have exactly one explicit request-stop call; found ${direct_stop_count}")
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
