if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/media/hdd.c" hdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/media/hdd_interface.h" hdd_header)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/media/fdd.c" fdd_source)

foreach(retired IN ITEMS fdd.c fdd.h fdd_private.h hdd.c hdd.h hdd_private.h media.h)
    if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/media/${retired}")
        message(FATAL_ERROR "Retired App media implementation remains: ${retired}")
    endif()
endforeach()
file(GLOB_RECURSE app_sources "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/*.h")
foreach(app_source IN LISTS app_sources)
    file(READ "${app_source}" source)
    if(source MATCHES "ibmpc/machine/media/(fdd|hdd)\\.h")
        message(FATAL_ERROR "App imports Shared-private media layout: ${app_source}")
    endif()
endforeach()

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
        "lib_storage_image" "fopen"
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
