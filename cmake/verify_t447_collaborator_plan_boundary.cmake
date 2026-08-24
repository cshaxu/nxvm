if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(project_t447_s6_contracts
    "src/core/machine/media_interface.h|core_machine_media_registry"
    "src/core/machine/display_interface.h|core_machine_display_provider_slot"
    "src/core/machine/machine_interface.h|core_machine_plan")

foreach(project_t447_s6_contract IN LISTS project_t447_s6_contracts)
    string(REPLACE "|" ";" project_t447_s6_parts "${project_t447_s6_contract}")
    list(GET project_t447_s6_parts 0 project_t447_s6_header)
    list(GET project_t447_s6_parts 1 project_t447_s6_type)
    file(READ "${PROJECT_SOURCE_DIR}/${project_t447_s6_header}" project_t447_s6_source)
    if(NOT project_t447_s6_source MATCHES
            "typedef struct ${project_t447_s6_type} ${project_t447_s6_type};")
        message(FATAL_ERROR "T447 S6 contract lacks opaque declaration: ${project_t447_s6_type}")
    endif()
    if(project_t447_s6_source MATCHES
            "struct ${project_t447_s6_type}[ \t\r\n]*\\{")
        message(FATAL_ERROR "T447 S6 public contract exposes layout: ${project_t447_s6_type}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_interface.h"
    project_t447_s6_machine_contract)
foreach(project_t447_s6_retired
        "core_machine_plan_memory_device"
        "core_machine_fdc_topology"
        "core_machine_hdc_topology"
        "d4_memory_parity_mask")
    if(project_t447_s6_machine_contract MATCHES "${project_t447_s6_retired}")
        message(FATAL_ERROR "T447 S6 public plan retains ${project_t447_s6_retired}")
    endif()
endforeach()

message("M5:T447:S6:COLLABORATOR-PLAN-BOUNDARY:OK")
