if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/platform/input_interface.h" input_header)
file(READ "${PROJECT_SOURCE_DIR}/src/core/platform/input.c" input_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/runner.c" runner_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32.c" win32_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32con.c" win32con_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32app.c" win32app_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/linux/linuxcon.c" linuxcon_source)

string(FIND "${input_header}" "request_stop" input_callback_position)
string(FIND "${input_source}" "vm_platform_keyboard_request_stop_for"
    input_operation_position)
string(FIND "${lifecycle_source}" "vm_session_keyboard_request_stop"
    session_callback_position)
if(NOT input_callback_position EQUAL -1 OR
   NOT input_operation_position EQUAL -1 OR
   NOT session_callback_position EQUAL -1)
    message(FATAL_ERROR "Keyboard transport still exposes lifecycle stop control")
endif()

foreach(source_text IN ITEMS "${win32_source}" "${win32con_source}" "${win32app_source}" "${linuxcon_source}")
    string(FIND "${source_text}" "vm_session_control_" session_control_position)
    string(FIND "${source_text}" "core_machine_request_stop" core_stop_position)
    if(NOT session_control_position EQUAL -1 OR NOT core_stop_position EQUAL -1)
        message(FATAL_ERROR "Platform source directly controls guest execution")
    endif()
endforeach()

string(FIND "${win32_source}" "virtualKey == VK_F9" win32_f9_position)
string(FIND "${linuxcon_source}" "if (keyvalue == KEY_F(9))" linux_f9_position)
if(win32_f9_position EQUAL -1 OR linux_f9_position EQUAL -1)
    message(FATAL_ERROR "A platform F9 path does not report through the run handle")
endif()
string(SUBSTRING "${win32_source}" ${win32_f9_position} -1 win32_f9_block)
string(SUBSTRING "${linuxcon_source}" ${linux_f9_position} -1 linux_f9_block)
foreach(f9_block IN ITEMS "${win32_f9_block}" "${linux_f9_block}")
    string(FIND "${f9_block}" "vm_platform_run_handle_report" report_call_position)
    string(FIND "${f9_block}" "VM_PLATFORM_RUN_EVENT_STOP_REQUESTED"
        report_event_position)
    if(report_call_position EQUAL -1 OR report_event_position EQUAL -1)
        message(FATAL_ERROR "A platform F9 path does not report through the run handle")
    endif()
endforeach()

foreach(required_source IN ITEMS "${win32con_source}" "${win32app_source}")
    string(FIND "${required_source}" "handle->owner" owner_position)
    if(owner_position EQUAL -1)
        message(FATAL_ERROR "Win32 backend does not pass its live run-handle borrow")
    endif()
endforeach()

string(FIND "${runner_source}" "vm_platform_run_handle_take_stop_report"
    consume_position)
string(FIND "${runner_source}" "vm_session_control_stop"
    stop_position)
if(consume_position EQUAL -1 OR stop_position EQUAL -1)
    message(FATAL_ERROR "Runner no longer owns report-to-session stop consumption")
endif()

message("M5:T201:S3:HOST-CANCELLATION-BOUNDARY:OK")
