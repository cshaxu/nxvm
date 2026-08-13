if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/vadp.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/vadp.h" vadp_header)

string(FIND "${vadp_header}" "#define CORE_MACHINE_VADP_CRTC_REGISTER_COUNT 20u" count_position)
if(count_position EQUAL -1)
    message(FATAL_ERROR "T314 CRTC register count is not 20")
endif()

foreach(required IN ITEMS
    "#define CORE_MACHINE_VADP_CRTC_OFFSET 0x13u"
    "_Static_assert(CORE_MACHINE_VADP_CRTC_CURSOR_TOP <"
    "CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM <"
    "CORE_MACHINE_VADP_CRTC_START_HIGH + 1u <"
    "CORE_MACHINE_VADP_CRTC_CURSOR_HIGH + 1u <"
    "CORE_MACHINE_VADP_CRTC_OFFSET < CORE_MACHINE_VADP_CRTC_REGISTER_COUNT"
    "return index < CORE_MACHINE_VADP_CRTC_REGISTER_COUNT &&"
    "adapter->data.crtc[high_index]"
    "adapter->data.crtc[low_index]"
    "adapter->data.crtc[adapter->data.crtc_index]")
    string(FIND "${vadp_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T314 CRTC boundary is missing ${required}")
    endif()
endforeach()

string(FIND "${vadp_source}" "!core_machine_vadp_supported_crtc_index(high_index) ||" word_guard)
string(FIND "${vadp_source}" "!core_machine_vadp_supported_crtc_index(low_index)" low_guard)
if(word_guard EQUAL -1 OR low_guard EQUAL -1)
    message(FATAL_ERROR "T314 CRTC word access is not predicate guarded")
endif()

string(FIND "${vadp_source}" "core_machine_vadp_supported_crtc_index(\n        adapter->data.crtc_index) ?" read_guard)
string(FIND "${vadp_source}" "!core_machine_vadp_supported_crtc_index(adapter->data.crtc_index))" write_guard)
if(read_guard EQUAL -1 OR write_guard EQUAL -1)
    message(FATAL_ERROR "T314 port CRTC access is not predicate guarded")
endif()

set(known_bypass "adapter->data.crtc[index]")
if(NOT known_bypass MATCHES "crtc\\[[a-z][a-z_]*\\]")
    message(FATAL_ERROR "T314 verifier self-test no longer recognizes a dynamic CRTC bypass")
endif()

set(dynamic_scan "${vadp_source}")
foreach(allowed IN ITEMS
    "adapter->data.crtc[high_index]"
    "adapter->data.crtc[low_index]"
    "adapter->data.crtc[adapter->data.crtc_index]")
    string(REPLACE "${allowed}" "" dynamic_scan "${dynamic_scan}")
endforeach()
if(dynamic_scan MATCHES "crtc\\[[a-z][a-z_]*\\]" OR
    dynamic_scan MATCHES "crtc\\[adapter")
    message(FATAL_ERROR "T314 found an unclassified dynamic CRTC subscript")
endif()

message("M5:T314:S6:EGA-CRTC-BOUNDARY:OK")
