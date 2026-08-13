if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/hdd.c" hdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/hdd.h" hdd_header)

foreach(forbidden IN ITEMS "pCurrByte" "transCount"
    "vm_machine_hdd_set_pointer" "vm_machine_hdd_transfer_read"
    "vm_machine_hdd_transfer_write" "vm_machine_hdd_format_track")
    string(FIND "${hdd_source}" "${forbidden}" source_position)
    string(FIND "${hdd_header}" "${forbidden}" header_position)
    if(NOT source_position EQUAL -1 OR NOT header_position EQUAL -1)
        message(FATAL_ERROR "HDD retains obsolete CHS transfer state: ${forbidden}")
    endif()
endforeach()

message("M5:T283:S4:HDD-LEGACY-API-CLOSURE:OK")
