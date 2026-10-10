if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(project_collaborator_plan_contracts
    "src/core/board-base/media_interface.h|core_machine_media_registry"
    "src/core/board-base/display_interface.h|core_machine_display_provider_slot"
    "src/core/board-base/machine_board_interface.h|core_machine_plan")

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

file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/machine_board_interface.h"
    project_collaborator_plan_machine_contract)
foreach(project_collaborator_plan_retired
        "core_machine_plan_memory_device"
        "core_machine_fdc_topology"
        "core_machine_hdc_topology"
        "d4_memory_parity_mask")
    if(project_collaborator_plan_machine_contract MATCHES "${project_collaborator_plan_retired}")
        message(FATAL_ERROR "public plan retains ${project_collaborator_plan_retired}")
    endif()
endforeach()

message("COLLABORATOR-PLAN-BOUNDARY:OK")
