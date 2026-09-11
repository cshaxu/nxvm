if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/lifecycle.c" source)

if(source MATCHES "vm_platform_|run_handle")
        message(FATAL_ERROR "VM machine lifecycle retains a platform run-handle path")
endif()

foreach(helper IN ITEMS vm_machine_execution_stop vm_machine_execution_join)
    string(FIND "${source}" "${helper}" helper_position)
    if(helper_position EQUAL -1)
        message(FATAL_ERROR "VM machine lifecycle helper is missing: ${helper}")
    endif()
endforeach()

message("M5:T526:MACHINE-LIFECYCLE-BOUNDARY:OK")
