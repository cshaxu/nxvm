if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(project_collaborator_plan_contracts
    "src/x86-nxvm/board-base/media_interface.h|x86_nxvm_board_media_registry"
    "src/x86-nxvm/board-base/display_interface.h|x86_nxvm_board_display_provider_slot"
    "src/x86-nxvm/board-base/machine_board_interface.h|x86_nxvm_board_plan")

foreach(project_collaborator_plan_contract IN LISTS project_collaborator_plan_contracts)
    string(REPLACE "|" ";" project_collaborator_plan_parts "${project_collaborator_plan_contract}")
    list(GET project_collaborator_plan_parts 0 project_collaborator_plan_header)
    list(GET project_collaborator_plan_parts 1 project_collaborator_plan_type)
    file(READ "${PROJECT_SOURCE_DIR}/${project_collaborator_plan_header}" project_collaborator_plan_source)
    if(NOT project_collaborator_plan_source MATCHES
            "typedef struct ${project_collaborator_plan_type} ${project_collaborator_plan_type};")
        message(FATAL_ERROR "contract lacks opaque declaration: ${project_collaborator_plan_type}")
    endif()
    if(project_collaborator_plan_source MATCHES
            "struct ${project_collaborator_plan_type}[ \t\r\n]*\\{")
        message(FATAL_ERROR "public contract exposes layout: ${project_collaborator_plan_type}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/machine_board_interface.h"
    project_collaborator_plan_machine_contract)
foreach(project_collaborator_plan_retired
        "core_machine_plan_memory_device"
        "x86_nxvm_board_fdc_topology"
        "x86_nxvm_board_hdc_topology"
        "d4_memory_parity_mask")
    if(project_collaborator_plan_machine_contract MATCHES "${project_collaborator_plan_retired}")
        message(FATAL_ERROR "public plan retains ${project_collaborator_plan_retired}")
    endif()
endforeach()

message("COLLABORATOR-PLAN-BOUNDARY:OK")
