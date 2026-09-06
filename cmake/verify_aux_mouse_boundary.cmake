if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/guest_input_interface.h" input_header)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/guest_input.c" input_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32.c" win32_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/mouse_mapper.c" mapper_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c" session_source)

string(FIND "${input_header}" "CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE" transport_position)
string(FIND "${input_source}" "core_machine_guest_input_source_submit" dispatch_position)
if(transport_position EQUAL -1 OR dispatch_position EQUAL -1)
    message(FATAL_ERROR "The host mouse transport surface is incomplete")
endif()

string(FIND "${win32_source}" "vm_platform_host_input_sink_submit"
    win32_submit_position)
string(FIND "${win32_source}" "vm_platform_win32_mouse_relative_for"
    win32_mouse_position)
if(win32_submit_position EQUAL -1 OR win32_mouse_position EQUAL -1)
    message(FATAL_ERROR "Win32 mouse events bypass the platform transport")
endif()

foreach(source_text IN ITEMS "${win32_source}" "${mapper_source}")
    foreach(forbidden IN ITEMS "qdkeyb" "INT 33" "BDA")
        string(FIND "${source_text}" "${forbidden}" forbidden_position)
        if(NOT forbidden_position EQUAL -1)
            message(FATAL_ERROR "Host mouse path contains forbidden guest shortcut: ${forbidden}")
        endif()
    endforeach()
endforeach()

string(FIND "${mapper_source}" "core_machine_" mapper_core_position)
if(NOT mapper_core_position EQUAL -1)
    message(FATAL_ERROR "Profile mouse mapper bypasses the platform transport")
endif()

string(FIND "${lifecycle_source}" "core_machine_guest_input_source_create"
    source_position)
string(FIND "${session_source}" "core_machine_guest_input_source_submit"
    ingress_position)
string(FIND "${lifecycle_source}" "VM_PLATFORM_REQUEST_MOUSE_EVENT"
    request_position)
string(FIND "${session_source}" "core_machine_mouse_receive_relative"
    consume_position)
if(source_position EQUAL -1 OR ingress_position EQUAL -1 OR
        request_position EQUAL -1 OR consume_position EQUAL -1)
    message(FATAL_ERROR "Session no longer owns mouse ingress consumption")
endif()

message("M5:T249:S2:AUX-MOUSE-BOUNDARY:OK")
