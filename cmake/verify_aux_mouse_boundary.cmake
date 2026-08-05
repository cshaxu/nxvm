if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/input.h" input_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/input.c" input_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32.c" win32_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32app.c" win32app_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/mouse_mapper.c" mapper_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c" session_source)

string(FIND "${input_header}" "vm_platform_mouse_transport" transport_position)
string(FIND "${input_source}" "receive_relative_event" dispatch_position)
if(transport_position EQUAL -1 OR dispatch_position EQUAL -1)
    message(FATAL_ERROR "The host mouse transport surface is incomplete")
endif()

string(FIND "${win32_source}" "vm_platform_mouse_receive_relative_event_for"
    win32_submit_position)
string(FIND "${win32app_source}" "vm_platform_win32_mouse_relative_for"
    win32app_submit_position)
if(win32_submit_position EQUAL -1 OR win32app_submit_position EQUAL -1)
    message(FATAL_ERROR "Win32 mouse events bypass the platform transport")
endif()

foreach(source_text IN ITEMS "${win32_source}" "${win32app_source}" "${mapper_source}")
    foreach(forbidden IN ITEMS "core_machine_" "qdkeyb" "INT 33" "BDA")
        string(FIND "${source_text}" "${forbidden}" forbidden_position)
        if(NOT forbidden_position EQUAL -1)
            message(FATAL_ERROR "Host mouse path contains forbidden guest shortcut: ${forbidden}")
        endif()
    endforeach()
endforeach()

string(FIND "${lifecycle_source}" "VM_PLATFORM_REQUEST_MOUSE_EVENT"
    request_position)
string(FIND "${session_source}" "core_machine_mouse_submit_relative"
    consume_position)
if(request_position EQUAL -1 OR consume_position EQUAL -1)
    message(FATAL_ERROR "Session no longer owns mouse ingress consumption")
endif()

message("M5:T229:S3:AUX-MOUSE-BOUNDARY:OK")
