cmake_minimum_required(VERSION 3.23)
if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED WORK)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR and WORK are required")
endif()

# Mutate only copied inputs in the owned build-tree fixture.
file(REMOVE_RECURSE "${WORK}")
set(cpu_files cpu.c cpu.h cpu_interface.h cpu_instructions.c cpu_instructions.h
    cpu_timing.c cpu_timing.h cpu_timing_model.c cpu_trace.h)
set(cpu_negative_files ${cpu_files})
list(REMOVE_ITEM cpu_negative_files cpu_trace.h)
set(paths src/core/machine/machine.c src/core/x86/machine.c
    src/core/x86/cpu_bus.c src/core/board-base/board_advance.c
    test/core/board-base/core_machine_lea_smoke.c
    test/core/board-base/core_machine_gpr_mov_smoke.c
    test/core/board-base/core_machine_moffs_smoke.c
    test/core/board-base/core_machine_xchg_smoke.c
    test/core/board-base/core_machine_gpr_push_pop_smoke.c
    test/core/board-base/core_machine_push_immediate_smoke.c
    test/core/board-base/core_machine_pusha_popa_smoke.c
    test/core/board-base/core_machine_enter_leave_smoke.c
    test/core/board-base/composition/core_machine_fs_gs_stack_smoke.c
    test/core/board-base/core_machine_legacy_sreg_stack_smoke.c
    test/core/board-base/composition/core_machine_movx_smoke.c
    test/core/board-base/core_machine_les_lds_board_smoke.c
    test/core/board-base/core_machine_les_lds_smoke.c
    test/core/board-base/core_machine_lss_lfs_lgs_smoke.c
    test/core/board-base/composition/core_machine_segment_selector_smoke.c
    test/core/board-base/core_machine_sreg_mov_smoke.c
    test/core/board-base/core_machine_bit_scan_smoke.c
    test/core/board-base/core_machine_bit_test_smoke.c
    test/core/board-base/core_machine_double_shift_smoke.c
    test/core/board-base/core_machine_imul2_smoke.c
    test/core/board-base/core_machine_setcc_smoke.c
    test/core/board-base/core_machine_sign_extend_smoke.c
    test/core/board-base/composition/core_machine_operand_address_smoke.c
    test/core/board-base/core_machine_prefix_attributes_smoke.c)
foreach(name IN LISTS cpu_files)
    list(APPEND paths "src/core/chips/cpu/${name}")
endforeach()
foreach(path IN LISTS paths)
    get_filename_component(directory "${WORK}/${path}" DIRECTORY)
    file(MAKE_DIRECTORY "${directory}")
    configure_file("${PROJECT_SOURCE_DIR}/${path}" "${WORK}/${path}" COPYONLY)
endforeach()
set(gate "${PROJECT_SOURCE_DIR}/cmake/nxvm/verify_core_cpu_pic_authority.cmake")
function(write_negative path original injection)
    if(injection MATCHES "^#include")
        file(WRITE "${path}" "${injection}\n${original}")
    else()
        file(WRITE "${path}" "${original}\n${injection}\n")
    endif()
endfunction()
execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
    -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "Unmodified CPU fixture rejected: ${output}${error}")
endif()

# Each pair covers one CPU file and one forbidden-dependency class.  The
# static gate still reads every CPU file, so this is an orthogonal matrix, not
# a reduction in the dependencies the gate must reject.
set(cpu_negative_tokens t_ram t_port core_machine_transaction
    firmware_interrupt software_interrupt core_machine_pic_scan_interrupt
    "#include \"core/x86/machine.h\""
    "#include \"outer/cpu_bus.h\"")
list(LENGTH cpu_negative_files cpu_negative_file_count)
list(LENGTH cpu_negative_tokens cpu_negative_token_count)
if(NOT cpu_negative_file_count EQUAL cpu_negative_token_count)
    message(FATAL_ERROR "CPU negative matrix must pair every file with one dependency class")
endif()
math(EXPR cpu_negative_last "${cpu_negative_file_count} - 1")
foreach(index RANGE 0 ${cpu_negative_last})
    list(GET cpu_negative_files ${index} name)
    list(GET cpu_negative_tokens ${index} token)
    set(path "${WORK}/src/core/chips/cpu/${name}")
    file(READ "${path}" original)
    if(token MATCHES "^#include")
        set(expected "CPU imports outside its neutral boundary")
    elseif(token MATCHES "^core_machine_pic_")
        set(expected "CPU retains a concrete PIC dependency")
    else()
        set(expected "CPU retains a board or firmware dependency")
    endif()
    write_negative("${path}" "${original}" "${token}")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
        -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    file(WRITE "${path}" "${original}")
    string(FIND "${output}${error}" "${expected}" match)
    if(status EQUAL 0 OR match EQUAL -1)
        message(FATAL_ERROR "CPU negative ${name}/${token} not specifically rejected: ${output}${error}")
    endif()
endforeach()
message(STATUS "CPU bus boundary: baseline and eight orthogonal controls pass")

set(path "${WORK}/src/core/x86/cpu_bus.c")
file(READ "${path}" original)
foreach(injection IN ITEMS "machine->executor_cpu.data.eax = 0\;"
        "machine->executor_cpu_instructions.data.except = 0\;"
        "machine->executor_cpu_execution.cpu = 0\;"
        "#include \"core/chips/cpu/cpu.h\""
        "#include \"core/chips/cpu/cpu_instructions.h\"")
    if(injection MATCHES "^#include")
        set(expected "Board imports private CPU layout")
    else()
        set(expected "Board bypasses copied CPU operations")
    endif()
    write_negative("${path}" "${original}" "${injection}")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
        -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    file(WRITE "${path}" "${original}")
    string(FIND "${output}${error}" "${expected}" match)
    if(status EQUAL 0 OR match EQUAL -1)
        message(FATAL_ERROR "Board negative ${injection} not specifically rejected: ${output}${error}")
    endif()
endforeach()
message(STATUS "Board CPU observation boundary: five negative controls pass")

set(board_test_negative_files core_machine_lea_smoke.c
    core_machine_movx_smoke.c core_machine_bit_scan_smoke.c
    core_machine_bit_test_smoke.c)
set(board_test_negative_tokens "machine->executor_cpu.data.eax = 0\;"
    "#include \"support/machine_cpu_fixture.h\""
    "#include \"core/chips/cpu/cpu.h\""
    "#include \"core/chips/cpu/cpu_instructions.h\"")
set(board_test_negative_index 0)
foreach(name IN LISTS board_test_negative_files)
    list(GET board_test_negative_tokens ${board_test_negative_index} injection)
    math(EXPR board_test_negative_index
        "(${board_test_negative_index} + 1) % 4")
    set(path "${WORK}/test/core/board-base/${name}")
    if(NOT EXISTS "${path}")
        set(path "${WORK}/test/core/board-base/composition/${name}")
    endif()
    file(READ "${path}" original)
    write_negative("${path}" "${original}" "${injection}")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DPROJECT_SOURCE_DIR=${WORK}"
        -P "${gate}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    file(WRITE "${path}" "${original}")
    string(FIND "${output}${error}" "Migrated board test bypasses CPU boundary" match)
    if(status EQUAL 0 OR match EQUAL -1)
        message(FATAL_ERROR "Migrated board negative ${name}/${injection} not rejected: ${output}${error}")
    endif()
endforeach()
message(STATUS "Migrated board tests: four dependency-class controls pass")
