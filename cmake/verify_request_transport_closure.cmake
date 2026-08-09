if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/vm_request_transport.h"
    transport_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/vm_request_transport.c"
    transport_source)
file(READ "${PROJECT_SOURCE_DIR}/docs/etc/architecture/machine-contract-details.md" contract_record)

foreach(source_text IN ITEMS "${transport_header}" "${transport_source}")
    string(FIND "${source_text}" "egress" egress_position)
    if(NOT egress_position EQUAL -1)
        message(FATAL_ERROR "Request transport still exposes outbound egress state or API")
    endif()
endforeach()

string(FIND "${contract_record}" "closes its ingress" ingress_position)
string(FIND "${contract_record}" "machine stop" stop_position)
if(ingress_position EQUAL -1 OR stop_position EQUAL -1)
    message(FATAL_ERROR "Request transport lifecycle contract is missing")
endif()

message("M5:T199:S1:REQUEST-TRANSPORT-CLOSURE:OK")
