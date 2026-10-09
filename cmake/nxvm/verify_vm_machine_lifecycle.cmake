if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/lifecycle.c" source)
file(READ "${PROJECT_SOURCE_DIR}/src/product/surface/composition.c" app_source)

if(source MATCHES "vm_platform_|run_handle|executor_fifo")
        message(FATAL_ERROR "VM machine lifecycle retains a platform run-handle path")
endif()

foreach(helper IN ITEMS
    emulator_machine_start
    emulator_machine_pause
    emulator_machine_reset
    emulator_machine_resume
    emulator_machine_stop)
    string(FIND "${source}" "${helper}" helper_position)
    if(helper_position EQUAL -1)
        message(FATAL_ERROR "VM machine Common lifecycle helper is missing: ${helper}")
    endif()
endforeach()

foreach(helper IN ITEMS emulator_machine_create app->factory.prepare
    app->factory.bind)
    string(FIND "${app_source}" "${helper}" helper_position)
    if(helper_position EQUAL -1)
        message(FATAL_ERROR "App Common composition helper is missing: ${helper}")
    endif()
endforeach()

if(source MATCHES "emulator_machine_create")
    message(FATAL_ERROR "VM machine lifecycle still constructs Common")
endif()

message("M5:T526:MACHINE-LIFECYCLE-BOUNDARY:OK")
