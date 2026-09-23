if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/guest_input_interface.h" input_header)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/machine.c" input_source)
file(READ "${PROJECT_SOURCE_DIR}/test/app-nxvm/unit/core/machine/vm_keyboard_host_ingress_smoke.c"
    input_smoke_source)

foreach(source_text IN ITEMS "${input_header}" "${input_source}" "${input_smoke_source}")
    foreach(forbidden "vm_platform_keyboard_modifier"
            "vm_platform_keyboard_get_modifier_for"
            "vm_machine_keyboard_get_modifier")
        string(FIND "${source_text}" "${forbidden}" forbidden_position)
        if(NOT forbidden_position EQUAL -1)
            message(FATAL_ERROR "VM keyboard transport still exposes modifier query: ${forbidden}")
        endif()
    endforeach()
endforeach()

string(FIND "${input_header}" "CORE_MACHINE_GUEST_INPUT_KEY" keypress_surface_position)
string(FIND "${input_source}" "vm_machine_submit_host_input"
    keypress_operation_position)
if(keypress_surface_position EQUAL -1 OR keypress_operation_position EQUAL -1 OR
   NOT input_smoke_source MATCHES "vm_machine_submit_host_input")
    message(FATAL_ERROR "VM keyboard transport lost its real keypress path")
endif()

message("M5:T249:S2:INPUT-TRANSPORT-SURFACE:OK")
