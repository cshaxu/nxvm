if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/machine_interface.h" input_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine.h" machine_header)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/machine_board_state.h" board_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine_scheduler.c" scheduler_source)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/machine.c" input_source)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/lifecycle.c" driver_source)
file(READ "${PROJECT_SOURCE_DIR}/test/app-nxvm/support/guest_input.h" fixture_source)
file(READ "${PROJECT_SOURCE_DIR}/test/app-nxvm/unit/core/machine/vm_keyboard_host_ingress_smoke.c"
    input_smoke_source)

foreach(instance IN ITEMS "t_kbc[ \t]+\\*[ \t]*shared_kbc;"
    "core_machine_xt_ppi_keyboard[ \t]+\\*[ \t]*xt_ppi_keyboard;")
    if(machine_header MATCHES "${instance}" OR NOT board_header MATCHES "${instance}")
        message(FATAL_ERROR "Keyboard instances must have one board owner")
    endif()
endforeach()
if(scheduler_source MATCHES "board->(shared_kbc|xt_ppi_keyboard)")
    message(FATAL_ERROR "Neutral scheduler directly owns board keyboard state")
endif()

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

string(FIND "${input_source}" "KVM_EVENT_KEY" keypress_surface_position)
string(FIND "${input_source}" "vm_machine_deliver_common_input"
    keypress_operation_position)
if(keypress_surface_position EQUAL -1 OR keypress_operation_position EQUAL -1 OR
   NOT driver_source MATCHES "vm_machine_deliver_common_input" OR
   NOT fixture_source MATCHES "common_machine_enqueue_input" OR
   NOT fixture_source MATCHES "vm_machine_deliver_common_input" OR
   NOT input_smoke_source MATCHES "vm_test_submit_host_input")
    message(FATAL_ERROR "VM keyboard transport lost its real keypress path")
endif()

message("M5:T249:S2:INPUT-TRANSPORT-SURFACE:OK")
