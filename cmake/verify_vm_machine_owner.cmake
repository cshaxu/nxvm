if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_text)
if(cmake_text MATCHES "vm-composition" OR
        cmake_text MATCHES "src/vm/composition/session")
    message(FATAL_ERROR "obsolete VM composition executor route remains in CMake")
endif()
if(EXISTS "${PROJECT_SOURCE_DIR}/src/vm/composition/session")
    message(FATAL_ERROR "obsolete VM composition executor source root remains")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/executor_fifo.c" fifo_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/event_interface.h" event_header)
foreach(required IN ITEMS
    "vm_machine_request"
    "VM_MACHINE_REQUEST_PAUSE"
    "VM_MACHINE_REQUEST_RESET"
    "VM_MACHINE_REQUEST_RESUME"
    "VM_MACHINE_REQUEST_STEP"
    "VM_MACHINE_REQUEST_STOP")
    string(FIND "${fifo_source}${event_header}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "machine executor FIFO lacks ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "core_machine *" "vm_machine *" "HWND" "HANDLE")
    string(FIND "${event_header}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "VM machine event ABI leaked an owner or native handle: ${forbidden}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/lifecycle.c" lifecycle_source)
foreach(forbidden IN ITEMS "set_lifecycle_reporter" "set_display_reporter")
    string(FIND "${lifecycle_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "obsolete split result callback remains: ${forbidden}")
    endif()
endforeach()
foreach(required IN ITEMS "vm_machine_set_result_sink" "vm_machine_publish_result")
    string(FIND "${lifecycle_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "machine copied-result route lacks ${required}")
    endif()
endforeach()

file(GLOB_RECURSE vm_sources
    "${PROJECT_SOURCE_DIR}/src/vm/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/*.h")
foreach(source IN LISTS vm_sources)
    file(RELATIVE_PATH relative "${PROJECT_SOURCE_DIR}/src/vm" "${source}")
    if(relative MATCHES "^(machine|profile)/")
        continue()
    endif()
    file(READ "${source}" source_text)
    if(source_text MATCHES "core_machine_(create|destroy|reset|run|request_stop|capture_display_snapshot)")
        message(FATAL_ERROR "direct Core runtime access outside vm/machine: ${relative}")
    endif()
endforeach()

message(STATUS "M5 VM machine owner and copied result boundary verified")
