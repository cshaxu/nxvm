if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/media/hdd.c" hdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/media/hdd.h" hdd_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/media/fdd.c" fdd_source)

foreach(forbidden IN ITEMS "pCurrByte" "transCount"
    "vm_machine_hdd_set_pointer" "vm_machine_hdd_transfer_read"
    "vm_machine_hdd_transfer_write" "vm_machine_hdd_format_track")
    string(FIND "${hdd_source}" "${forbidden}" source_position)
    string(FIND "${hdd_header}" "${forbidden}" header_position)
    if(NOT source_position EQUAL -1 OR NOT header_position EQUAL -1)
        message(FATAL_ERROR "HDD retains obsolete CHS transfer state: ${forbidden}")
    endif()
endforeach()

foreach(source_text IN ITEMS "${hdd_source}" "${fdd_source}")
    foreach(forbidden IN ITEMS "pImgBase" "lib_storage_file_read_owned"
        "lib_storage_image" "STD_FOPEN" "STD_FREAD" "STD_FWRITE" "fopen"
        "fread" "fwrite")
        string(FIND "${source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR
                "FDD/HDD retains a second media-file route: ${forbidden}")
        endif()
    endforeach()
    foreach(required IN ITEMS "lib_storage_medium_read_at"
        "lib_storage_medium_write_at")
        string(FIND "${source_text}" "${required}" position)
        if(position EQUAL -1)
            message(FATAL_ERROR
                "FDD/HDD omits the sole byte-medium route: ${required}")
        endif()
    endforeach()
endforeach()

message("M5:T524:S10:MEDIA-SOLE-ROUTE:OK")
