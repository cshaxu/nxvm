if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session_private.h" session_header)
foreach(forbidden IN ITEMS
    "core_machine_rtc *rtc;"
    "t_fdd *fdd;"
    "core_machine_fdc fdc;"
    "core_machine_hdc hdc;"
    "t_hdd *hdd;"
    "t_debug *debug;"
    "vm_session_control_state *control;")
    string(FIND "${session_header}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "VM session owner-alias closure rejected: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS
    "core_machine *core_machine;"
    "core_product_debug_target *debug_target;"
    "vm_session_control_state control;")
    string(FIND "${session_header}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "VM session owner-closure requirement missing: ${required}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c" session_source)
foreach(forbidden IN ITEMS
    "STD_CALLOC(1u,\n        sizeof(*machine->control))"
    "STD_FREE(machine->control)")
    string(FIND "${session_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "VM session control heap ownership remains")
    endif()
endforeach()

message(STATUS "M5 VM session owner-alias closure verified")
