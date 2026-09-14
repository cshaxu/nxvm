if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/lifecycle.c" source)

if(source MATCHES "vm_platform_|run_handle|executor_fifo")
        message(FATAL_ERROR "VM machine lifecycle retains a platform run-handle path")
endif()

foreach(helper IN ITEMS
    common_machine_create
    common_machine_start
    common_machine_pause
    common_machine_reset
    common_machine_resume
    common_machine_stop)
    string(FIND "${source}" "${helper}" helper_position)
    if(helper_position EQUAL -1)
        message(FATAL_ERROR "VM machine Common lifecycle helper is missing: ${helper}")
    endif()
endforeach()

message("M5:T526:MACHINE-LIFECYCLE-BOUNDARY:OK")
