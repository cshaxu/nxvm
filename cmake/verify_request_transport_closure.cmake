if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/vm_request_transport.h"
    transport_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/vm_request_transport.c"
    transport_source)
file(READ "${PROJECT_SOURCE_DIR}/docs/tracking/M5-T194.md" t194_record)

foreach(source_text IN ITEMS "${transport_header}" "${transport_source}")
    string(FIND "${source_text}" "egress" egress_position)
    if(NOT egress_position EQUAL -1)
        message(FATAL_ERROR "Request transport still exposes outbound egress state or API")
    endif()
endforeach()

string(FIND "${t194_record}" "run handle" run_handle_position)
string(FIND "${t194_record}" "kept out of request transport" transport_position)
if(run_handle_position EQUAL -1 OR transport_position EQUAL -1)
    message(FATAL_ERROR "T194 worker-cancellation record is stale")
endif()

message("M5:T199:S1:REQUEST-TRANSPORT-CLOSURE:OK")
