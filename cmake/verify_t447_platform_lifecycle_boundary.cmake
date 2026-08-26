if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(project_t447_s5_contracts
    "src/core/platform/input_interface.h|core_platform_input_source"
    "src/core/platform/presentation_mailbox_interface.h|core_platform_presentation_mailbox"
    "src/vm/platform/execution.h|vm_platform_execution_transport"
    "src/vm/platform/host_surface.h|vm_platform_host_surface_context"
    "src/vm/platform/host_surface.h|vm_platform_host_surface_lease"
    "src/vm/platform/platform.h|vm_platform_run_context"
    "src/vm/platform/platform.h|vm_platform_run_handle"
    "src/vm/platform/vm_request_transport.h|vm_platform_request_transport")

foreach(project_t447_s5_contract IN LISTS project_t447_s5_contracts)
    string(REPLACE "|" ";" project_t447_s5_parts "${project_t447_s5_contract}")
    list(GET project_t447_s5_parts 0 project_t447_s5_header)
    list(GET project_t447_s5_parts 1 project_t447_s5_type)
    file(READ "${PROJECT_SOURCE_DIR}/${project_t447_s5_header}" project_t447_s5_source)
    if(NOT project_t447_s5_source MATCHES
            "typedef struct ${project_t447_s5_type} ${project_t447_s5_type};")
        message(FATAL_ERROR "T447 S5 contract lacks opaque declaration: ${project_t447_s5_type}")
    endif()
    if(project_t447_s5_source MATCHES
            "struct ${project_t447_s5_type}[ \\t\\r\\n]*\\{")
        message(FATAL_ERROR "T447 S5 public contract exposes layout: ${project_t447_s5_type}")
    endif()
endforeach()

message("M5:T447:S5:PLATFORM-LIFECYCLE-BOUNDARY:OK")
