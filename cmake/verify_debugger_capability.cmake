if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE debugger_sources
    "${PROJECT_SOURCE_DIR}/src/vm/composition/session/*.[ch]"
    "${PROJECT_SOURCE_DIR}/src/vm/machine/debug.[ch]")
foreach(source IN LISTS debugger_sources)
    file(READ "${source}" contents)
    foreach(forbidden "core_machine_debug_cpu_borrow"
            "core_machine_debug_cpu_instructions_borrow"
            "core_machine_debug_cpu_execution_borrow"
            "core_machine_debug_memory_borrow"
            "core_machine_debug_port_borrow"
            "core_machine_configuration_cpu_borrow"
            "core_machine_configuration_cpu_instructions_borrow")
        string(FIND "${contents}" "${forbidden}" found)
        if(NOT found EQUAL -1)
            message(FATAL_ERROR
                "Debugger raw borrow remains in ${source}: ${forbidden}")
        endif()
    endforeach()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/debug_interface.h" contract)
foreach(required "core_machine_debug_capture_instruction_observation"
        "core_machine_debug_read_register" "core_machine_debug_patch_registers"
        "core_machine_debug_read_linear" "core_machine_debug_write_linear"
        "core_machine_debug_read_real" "core_machine_debug_write_real"
        "core_machine_debug_read_port" "core_machine_debug_write_port"
        "core_machine_debug_set_watchpoint"
        "core_machine_debug_clear_watchpoint")
    string(FIND "${contract}" "${required}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Debugger capability is missing ${required}")
    endif()
endforeach()

message("M5:T298:S3:DEBUGGER-CAPABILITY-STATIC:OK")
