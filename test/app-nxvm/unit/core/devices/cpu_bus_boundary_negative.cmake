cmake_minimum_required(VERSION 3.23)
if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED WORK)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR and WORK are required")
endif()

# Mutate only copied inputs in the owned build-tree fixture.
file(REMOVE_RECURSE "${WORK}")
set(cpu_files cpu.c cpu.h cpu_interface.h cpu_instructions.c cpu_instructions.h
    cpu_timing.c cpu_timing.h cpu_timing_model.c cpu_trace.h)
set(paths src/app-nxvm/machine/machine.c src/app-nxvm/devices/machine.c
    src/app-nxvm/devices/cpu_bus.c
    test/app-nxvm/unit/core/devices/core_machine_lea_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_gpr_mov_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_moffs_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_xchg_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_gpr_push_pop_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_push_immediate_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_pusha_popa_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_enter_leave_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_fs_gs_stack_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_legacy_sreg_stack_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_movx_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_les_lds_s41_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_les_lds_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_lss_lfs_lgs_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_segment_selector_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_sreg_mov_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_bit_scan_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_bit_test_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_double_shift_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_imul2_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_setcc_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_sign_extend_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_operand_address_smoke.c
    test/app-nxvm/unit/core/devices/core_machine_prefix_attributes_s64_smoke.c)
foreach(name IN LISTS cpu_files)
    list(APPEND paths "src/x86/devices/cpu/${name}")
endforeach()
foreach(path IN LISTS paths)
    get_filename_component(directory "${WORK}/${path}" DIRECTORY)
    file(MAKE_DIRECTORY "${directory}")
    configure_file("${PROJECT_SOURCE_DIR}/${path}" "${WORK}/${path}" COPYONLY)
endforeach()
set(gate "${PROJECT_SOURCE_DIR}/cmake/nxvm/verify_core_cpu_pic_authority.cmake")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
    -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "Unmodified CPU fixture rejected: ${output}${error}")
endif()

foreach(name IN LISTS cpu_files)
    set(path "${WORK}/src/x86/devices/cpu/${name}")
    file(READ "${path}" original)
    foreach(token IN ITEMS t_ram t_port core_machine_transaction
            firmware_interrupt software_interrupt core_machine_pic_scan_interrupt
            "#include \"app-nxvm/devices/machine.h\""
            "#include \"app-nxvm/devices/cpu_bus.h\"")
        if(token MATCHES "^#include")
            set(expected "CPU imports outside its neutral boundary")
        elseif(token MATCHES "^core_machine_pic_")
            set(expected "CPU retains a concrete PIC dependency")
        else()
            set(expected "CPU retains a board or firmware dependency")
        endif()
        file(WRITE "${path}" "${original}\n${token}\n")
        execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
            -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
        file(WRITE "${path}" "${original}")
        string(FIND "${output}${error}" "${expected}" match)
        if(status EQUAL 0 OR match EQUAL -1)
            message(FATAL_ERROR "CPU negative ${name}/${token} not specifically rejected: ${output}${error}")
        endif()
    endforeach()
endforeach()
message(STATUS "CPU bus boundary: baseline and 72 negative controls pass")

set(path "${WORK}/src/app-nxvm/devices/cpu_bus.c")
file(READ "${path}" original)
foreach(injection IN ITEMS "machine->executor_cpu.data.eax = 0\;"
        "machine->executor_cpu_instructions.data.except = 0\;"
        "machine->executor_cpu_execution.cpu = 0\;"
        "#include \"x86/devices/cpu/cpu.h\""
        "#include \"x86/devices/cpu/cpu_instructions.h\"")
    if(injection MATCHES "^#include")
        set(expected "Board imports private CPU layout")
    else()
        set(expected "Board bypasses copied CPU operations")
    endif()
    file(WRITE "${path}" "${original}\n${injection}\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
        -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    file(WRITE "${path}" "${original}")
    string(FIND "${output}${error}" "${expected}" match)
    if(status EQUAL 0 OR match EQUAL -1)
        message(FATAL_ERROR "Board negative ${injection} not specifically rejected: ${output}${error}")
    endif()
endforeach()
message(STATUS "Board CPU observation boundary: five negative controls pass")

foreach(name core_machine_lea_smoke.c core_machine_movx_smoke.c
        core_machine_bit_scan_smoke.c
        core_machine_bit_test_smoke.c
        core_machine_double_shift_smoke.c core_machine_imul2_smoke.c
        core_machine_setcc_smoke.c
        core_machine_sign_extend_smoke.c
        core_machine_gpr_mov_smoke.c core_machine_moffs_smoke.c core_machine_xchg_smoke.c
        core_machine_gpr_push_pop_smoke.c core_machine_push_immediate_smoke.c core_machine_pusha_popa_smoke.c
        core_machine_enter_leave_smoke.c core_machine_fs_gs_stack_smoke.c
        core_machine_legacy_sreg_stack_smoke.c
        core_machine_les_lds_s41_smoke.c core_machine_les_lds_smoke.c
        core_machine_lss_lfs_lgs_smoke.c core_machine_segment_selector_smoke.c
        core_machine_sreg_mov_smoke.c
        core_machine_operand_address_smoke.c core_machine_prefix_attributes_s64_smoke.c)
    set(path "${WORK}/test/app-nxvm/unit/core/devices/${name}")
    file(READ "${path}" original)
    foreach(injection IN ITEMS "machine->executor_cpu.data.eax = 0\;"
            "#include \"support/machine_cpu_fixture.h\""
            "#include \"x86/devices/cpu/cpu.h\""
            "#include \"x86/devices/cpu/cpu_instructions.h\"")
        file(WRITE "${path}" "${original}\n${injection}\n")
        execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
            -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
        file(WRITE "${path}" "${original}")
        string(FIND "${output}${error}" "Migrated board test bypasses CPU boundary" match)
        if(status EQUAL 0 OR match EQUAL -1)
            message(FATAL_ERROR "Migrated board negative ${name}/${injection} not rejected: ${output}${error}")
        endif()
    endforeach()
endforeach()
message(STATUS "Migrated board tests: 96 negative controls pass")
