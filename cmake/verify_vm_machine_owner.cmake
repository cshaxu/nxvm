if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_text)
if(cmake_text MATCHES "vm-composition" OR
        cmake_text MATCHES "src/vm/composition/session")
    message(FATAL_ERROR "obsolete VM composition executor route remains in CMake")
endif()
if(EXISTS "${PROJECT_SOURCE_DIR}/src/vm/composition/session")
    message(FATAL_ERROR "obsolete VM composition executor source root remains")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/event_interface.h" event_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/app/composition.c" app_source)

# Common owns the sole lifecycle queue and worker.  NXVM supplies only the
# bounded Core driver and forwards Common's copied facts to Common Session.
foreach(required IN ITEMS
    "common_machine_create"
    "common_machine_start"
    "common_machine_pause"
    "common_machine_reset"
    "common_machine_resume"
    "common_machine_stop")
    string(FIND "${lifecycle_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "VM machine Common lifecycle route lacks ${required}")
    endif()
endforeach()

foreach(required IN ITEMS
    "common_machine_set_state_sink"
    "common_machine_set_frame_sink"
    "common_session_enqueue_runtime_completed"
    "common_session_enqueue_frame_completed")
    string(FIND "${app_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "VM app does not forward Common fact ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "vm_machine_result" "vm_machine_set_result_sink"
    "vm_machine_publish_result" "HWND" "HANDLE")
    string(FIND "${event_header}${lifecycle_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "obsolete VM result or native-handle route remains: ${forbidden}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "set_lifecycle_reporter" "set_display_reporter")
    string(FIND "${lifecycle_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "obsolete split result callback remains: ${forbidden}")
    endif()
endforeach()

file(GLOB_RECURSE vm_sources
    "${PROJECT_SOURCE_DIR}/src/vm/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/*.h")
foreach(source IN LISTS vm_sources)
    file(RELATIVE_PATH relative "${PROJECT_SOURCE_DIR}/src/vm" "${source}")
    if(relative MATCHES "^(machine|profile)/")
        continue()
    endif()
    file(READ "${source}" source_text)
    if(source_text MATCHES "core_machine_(create|destroy|reset|run|request_stop|capture_display_snapshot)")
        message(FATAL_ERROR "direct Core runtime access outside vm/machine: ${relative}")
    endif()
endforeach()

message(STATUS "M5 VM machine Common-owner and copied-fact boundary verified")
