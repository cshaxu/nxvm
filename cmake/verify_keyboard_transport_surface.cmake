if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/platform/input_interface.h" input_header)
file(READ "${PROJECT_SOURCE_DIR}/src/core/platform/input.c" input_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/test/core/platform/core_platform_input_source_smoke.c"
    input_smoke_source)

foreach(source_text IN ITEMS "${input_header}" "${input_source}"
        "${lifecycle_source}" "${input_smoke_source}")
    foreach(forbidden "vm_platform_keyboard_modifier"
            "vm_platform_keyboard_get_modifier_for"
            "vm_session_keyboard_get_modifier")
        string(FIND "${source_text}" "${forbidden}" forbidden_position)
        if(NOT forbidden_position EQUAL -1)
            message(FATAL_ERROR "VM keyboard transport still exposes modifier query: ${forbidden}")
        endif()
    endforeach()
endforeach()

string(FIND "${input_header}" "CORE_PLATFORM_INPUT_KEY" keypress_surface_position)
string(FIND "${input_source}" "core_platform_input_source_submit"
    keypress_operation_position)
string(FIND "${lifecycle_source}" "vm_session_input_submit"
    session_ingress_position)
if(keypress_surface_position EQUAL -1 OR keypress_operation_position EQUAL -1 OR
   session_ingress_position EQUAL -1)
    message(FATAL_ERROR "VM keyboard transport lost its real keypress path")
endif()

message("M5:T249:S2:INPUT-TRANSPORT-SURFACE:OK")
