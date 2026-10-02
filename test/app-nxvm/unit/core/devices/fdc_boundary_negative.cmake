cmake_minimum_required(VERSION 3.23)
if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED WORK)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR and WORK are required")
endif()

# An owned build-tree fixture, not edits to the source tree under audit.
set(paths
    src/app-nxvm/devices/fdc.c
    src/app-nxvm/devices/fdc.h
    src/app-nxvm/devices/machine_scheduler.c
    src/app-nxvm/devices/board_deadline.c
    src/app-nxvm/devices/machine_board.c
    src/app-nxvm/devices/dma_bus.c
    src/app-nxvm/devices/memory_interface.c
    src/app-nxvm/machine/machine_devices.c
    src/app-nxvm/machine/media/fdd.c
    src/app-nxvm/machine/media/fdd.h
    src/x86/chips/fdc8272/fdc.c
    src/x86/chips/fdc8272/fdc.h
    src/x86/chips/fdc8272/fdc8272_interface.h
    test/app-nxvm/unit/core/devices/core_machine_fdc_smoke.c)
foreach(path IN LISTS paths)
    get_filename_component(directory "${WORK}/${path}" DIRECTORY)
    file(MAKE_DIRECTORY "${directory}")
    configure_file("${PROJECT_SOURCE_DIR}/${path}" "${WORK}/${path}" COPYONLY)
endforeach()
set(fdc_gate "${PROJECT_SOURCE_DIR}/cmake/nxvm/verify_fdc_state_machine_boundary.cmake")
set(dma_gate "${PROJECT_SOURCE_DIR}/cmake/nxvm/verify_dma_fdc_boundary.cmake")
foreach(gate IN ITEMS "${fdc_gate}" "${dma_gate}")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
        -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(NOT status EQUAL 0)
        message(FATAL_ERROR "Unmodified FDC fixture rejected: ${output}${error}")
    endif()
endforeach()

foreach(case RANGE 0 6)
    set(gate "${fdc_gate}")
    if(case EQUAL 0)
        set(path src/x86/chips/fdc8272/fdc.c)
        set(injection "#include \"app-nxvm/devices/port.h\"")
        set(expected "Shared FDC retains board ownership")
    elseif(case EQUAL 1)
        set(path src/x86/chips/fdc8272/fdc8272_interface.h)
        set(injection "struct x86_fdc { lib_u8 phase; };")
        set(expected "Shared FDC public layout is not opaque")
    elseif(case EQUAL 2)
        set(path src/app-nxvm/devices/fdc.c)
        set(injection "void obsolete(void) { core_machine_fdc_PHASE_COMMAND; }")
        set(expected "NXVM crosses the opaque FDC boundary")
    elseif(case EQUAL 3)
        set(path src/app-nxvm/devices/machine_scheduler.c)
        set(injection "#include \"x86/chips/fdc8272/fdc.h\"")
        set(expected "NXVM crosses the opaque FDC boundary")
    elseif(case EQUAL 4)
        set(path src/app-nxvm/devices/fdc.c)
        set(injection "void bypass(void) { core_machine_dma_set_drq(); }")
        set(gate "${dma_gate}")
        set(expected "FDC retains forbidden raw DMA or RAM access")
    elseif(case EQUAL 5)
        set(path src/app-nxvm/machine/media/fdd.h)
        set(injection "lib_u32 transCount;")
        set(expected "FDD retains a controller-owned transfer cursor")
    else()
        set(path src/app-nxvm/devices/machine_scheduler.c)
        set(injection "void bypass(void) { machine->board->fdc.data.phase; }")
        set(expected "Core scheduler directly owns board FDC")
    endif()
    file(READ "${WORK}/${path}" original)
    file(APPEND "${WORK}/${path}" "\n${injection}\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
        -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    file(WRITE "${WORK}/${path}" "${original}")
    string(FIND "${output}${error}" "${expected}" match)
    if(status EQUAL 0 OR match EQUAL -1)
        message(FATAL_ERROR "FDC negative ${case} not specifically rejected: ${output}${error}")
    endif()
endforeach()
message(STATUS "FDC chip/board boundaries: baseline and seven negative controls pass")
