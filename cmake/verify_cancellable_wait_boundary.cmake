if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/platform/wait_interface.h" wait_header)
file(READ "${PROJECT_SOURCE_DIR}/src/core/platform/wait.c" wait_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/execution.c" execution_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32con.c" win32con_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32app.c" win32app_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/linux/linuxcon.c" linuxcon_source)

foreach(source_text IN ITEMS "${wait_header}" "${wait_source}")
    string(FIND "${source_text}" "core/machine" machine_include)
    string(FIND "${source_text}" "elapsed_ticks" guest_time)
    if(NOT machine_include EQUAL -1 OR NOT guest_time EQUAL -1)
        message(FATAL_ERROR "Core platform wait must remain host-only")
    endif()
endforeach()

foreach(source_text IN ITEMS "${execution_source}" "${win32con_source}"
        "${win32app_source}"
        "${linuxcon_source}")
    string(FIND "${source_text}" "core_platform_wait_milliseconds" wait_call)
    if(wait_call EQUAL -1)
        message(FATAL_ERROR "A classified lifecycle wait does not use the core boundary")
    endif()
endforeach()

message("M5:T251:S2:CANCELLABLE-WAIT-BOUNDARY:OK")
