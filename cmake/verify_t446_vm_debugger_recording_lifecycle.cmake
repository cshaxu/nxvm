if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/product/recorder.c" recorder_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/debug.c" machine_source)
foreach(required IN ITEMS
    "vm_product_recorder_start"
    "vm_product_recorder_stop"
    "vm_product_recorder_observe"
    "lib_storage_file_writer_open")
    string(FIND "${recorder_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Product recorder lacks ${required}")
    endif()
endforeach()
foreach(forbidden IN ITEMS
    "recordFile"
    "lib_storage_file_writer"
    "common_xasm32")
    string(FIND "${machine_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "vm/machine retains recorder policy: ${forbidden}")
    endif()
endforeach()

message(STATUS "M5 T527 product recorder ownership: OK")
