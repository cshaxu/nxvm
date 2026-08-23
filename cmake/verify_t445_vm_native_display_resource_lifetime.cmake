if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/w32adisp.c" win32_display)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32app.c" win32_app)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/linux/linuxcon.c" linux_console)

string(REGEX MATCHALL "GetDC\\(" get_dc_calls "${win32_display}")
list(LENGTH get_dc_calls get_dc_count)
string(REGEX MATCHALL "ReleaseDC\\(" release_dc_calls "${win32_display}")
list(LENGTH release_dc_calls release_dc_count)
if(NOT get_dc_count EQUAL 1 OR NOT release_dc_count EQUAL 1)
    message(FATAL_ERROR
        "Win32 display must retain one GetDC/ReleaseDC owner pair; found ${get_dc_count}/${release_dc_count}")
endif()

foreach(required
    "context->window = window;"
    "ReleaseDC(context->window, context->window_dc);"
    "context->window_dc = STD_NULL;")
    string(FIND "${win32_display}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Win32 display DC lifecycle is incomplete: ${required}")
    endif()
endforeach()

string(FIND "${win32_app}" "static DWORD WINAPI win32app_display_thread" display_thread_position)
string(FIND "${win32_app}" "C_VOID vm_platform_win32app_run_handle_finalize" outer_finalize_position)
if(display_thread_position EQUAL -1 OR outer_finalize_position EQUAL -1)
    message(FATAL_ERROR "Win32 display lifecycle owners are missing")
endif()
math(EXPR display_thread_length "${outer_finalize_position} - ${display_thread_position}")
string(SUBSTRING "${win32_app}" ${display_thread_position}
    ${display_thread_length} display_thread_source)
string(FIND "${display_thread_source}" "win32app_display_renderer_finalize(platform);"
    display_cleanup_position)
if(display_cleanup_position EQUAL -1)
    message(FATAL_ERROR "Win32 display thread does not finalize its renderer")
endif()
string(FIND "${win32_app}" "case WM_CLOSE:" close_message_position)
if(close_message_position EQUAL -1)
    message(FATAL_ERROR "Win32 window closes without a renderer cleanup point")
endif()
string(SUBSTRING "${win32_app}" ${outer_finalize_position} -1 outer_finalize_source)
foreach(forbidden "w32adispFinal" "w32adisp_context_destroy"
    "window_renderer = STD_NULL")
    string(FIND "${outer_finalize_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Win32 outer finalizer retains display-thread cleanup: ${forbidden}")
    endif()
endforeach()

string(FIND "${linux_console}" "terminal_initialized" terminal_state_position)
if(NOT terminal_state_position EQUAL -1)
    message(FATAL_ERROR "Linux console retains cross-thread terminal lifecycle state")
endif()
string(FIND "${linux_console}" "static C_VOID *linuxcon_display_thread" linux_display_position)
string(FIND "${linux_console}" "static C_VOID *linuxcon_kernel_thread" linux_kernel_position)
string(FIND "${linux_console}" "C_VOID vm_platform_linuxcon_run_handle_finalize" linux_finalize_position)
if(linux_display_position EQUAL -1 OR linux_kernel_position EQUAL -1 OR
        linux_finalize_position EQUAL -1)
    message(FATAL_ERROR "Linux console lifecycle owners are missing")
endif()
math(EXPR linux_display_length "${linux_kernel_position} - ${linux_display_position}")
string(SUBSTRING "${linux_console}" ${linux_display_position}
    ${linux_display_length} linux_display_source)
string(FIND "${linux_display_source}" "vm_platform_linuxcon_terminal_finalize();"
    terminal_cleanup_position)
if(terminal_cleanup_position EQUAL -1)
    message(FATAL_ERROR "Linux display thread does not finalize curses")
endif()
string(SUBSTRING "${linux_console}" ${linux_finalize_position} -1 linux_outer_finalize_source)
string(FIND "${linux_outer_finalize_source}" "vm_platform_linuxcon_terminal_finalize"
    terminal_outer_cleanup_position)
if(NOT terminal_outer_cleanup_position EQUAL -1)
    message(FATAL_ERROR "Linux outer finalizer retains display-thread curses cleanup")
endif()

message("M5:T445:S1:NATIVE-DISPLAY-RESOURCE-LIFETIME:OK")
