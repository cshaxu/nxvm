if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()
get_filename_component(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}" ABSOLUTE)

set(machine_source "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(memory_source "${PROJECT_SOURCE_DIR}/src/core/machine/memory.c")
set(machine_runtime_source "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/machine.c")

foreach(file IN ITEMS "${machine_source}" "${memory_source}" "${machine_runtime_source}")
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "M5 T174 missing RAM closure source: ${file}")
    endif()
endforeach()

file(READ "${machine_source}" machine)
file(READ "${memory_source}" memory)
file(READ "${machine_runtime_source}" machine_runtime)

foreach(required IN ITEMS "core_machine_reconfigure_memory"
    "CORE_MACHINE_STOPPED" "core_machine_cold_reset")
    string(FIND "${machine}" "${required}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "M5 T174 missing core RAM contract: ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "core_machine_memory_real_address" "pBase")
    string(FIND "${memory}" "${forbidden}" found)
    if(NOT found EQUAL -1)
        message(FATAL_ERROR "M5 T174 obsolete RAM access remains: ${forbidden}")
    endif()
endforeach()

string(FIND "${machine_runtime}" "core_machine_reconfigure_memory" core_route)
string(FIND "${machine_runtime}" "STD_MEMSET(machine, 0, sizeof(*machine))" machine_rebuild)
if(core_route EQUAL -1 OR NOT machine_rebuild EQUAL -1)
    message(FATAL_ERROR "M5 T174 VM machine RAM route is not core-owned")
endif()

file(GLOB_RECURSE source_files "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.h")
foreach(file IN LISTS source_files)
    file(RELATIVE_PATH relative "${PROJECT_SOURCE_DIR}/src/core/machine" "${file}")
    string(FIND "${relative}" "../" parent_prefix)
    if(NOT parent_prefix EQUAL 0)
        continue()
    endif()
    file(READ "${file}" source)
    foreach(forbidden IN ITEMS "connect.pBase" "connect.backing")
        string(FIND "${source}" "${forbidden}" found)
        if(NOT found EQUAL -1)
            message(FATAL_ERROR
                "M5 T174 non-core RAM backing access remains: ${file}: ${forbidden}")
        endif()
    endforeach()
endforeach()

message(STATUS "M5 T174 RAM cold-reconfiguration closure: OK")
