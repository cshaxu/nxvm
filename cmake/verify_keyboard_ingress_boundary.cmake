if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/input.h" input_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/input.c" input_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32.c" win32_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c" session_source)

foreach(source_text IN ITEMS "${input_header}" "${input_source}" "${lifecycle_source}")
    string(FIND "${source_text}" "apply_host_state" direct_state_position)
    if(NOT direct_state_position EQUAL -1)
        message(FATAL_ERROR "Historical direct VM keyboard-state callback remains")
    endif()
endforeach()

string(FIND "${win32_source}" "vm_platform_run_context_submit_keyboard_state"
    win32_submit_position)
string(FIND "${win32_source}" "keyboard_state_sink" win32_direct_position)
if(win32_submit_position EQUAL -1 OR NOT win32_direct_position EQUAL -1)
    message(FATAL_ERROR "Win32 keyboard host state bypasses the run-context ingress operation")
endif()

string(FIND "${session_source}" "VM_PLATFORM_REQUEST_KEYBOARD_STATE"
    request_position)
string(FIND "${session_source}" "core_machine_keyboard_apply_host_state_to"
    consume_position)
if(request_position EQUAL -1 OR consume_position EQUAL -1)
    message(FATAL_ERROR "Session no longer owns keyboard-state ingress consumption")
endif()

message("M5:T200:S1:KEYBOARD-INGRESS-BOUNDARY:OK")
