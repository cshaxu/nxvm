if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/cmake/nxvm/NxvmProduct.cmake" cmake_source)
file(READ "${PROJECT_SOURCE_DIR}/CMakePresets.json" presets_source)

foreach(forbidden "add_vm_task_artifact" "add_vm_version_artifact")
    string(FIND "${cmake_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Historical artifact target helper remains: ${forbidden}")
    endif()
endforeach()

if(NOT DEFINED PROJECT_CURRENT_ARTIFACT_GRAPH OR
        NOT EXISTS "${PROJECT_CURRENT_ARTIFACT_GRAPH}")
    message(FATAL_ERROR "The configured selected-product graph is required")
endif()
file(READ "${PROJECT_CURRENT_ARTIFACT_GRAPH}" graph)
string(REPLACE "\n" "" graph "${graph}")
set(current_targets)
foreach(target IN LISTS graph)
    if(target MATCHES "^vm-[0-9]+-[0-9]+-[0-9]+$")
        list(APPEND current_targets "${target}")
    endif()
endforeach()
list(LENGTH current_targets current_target_count)
if(NOT current_target_count EQUAL 1)
    message(FATAL_ERROR
        "Expected exactly one current artifact target; found ${current_target_count}")
endif()
list(GET current_targets 0 current_target)
string(FIND "${cmake_source}" "add_current_vm_artifact(${current_target} " source_position)
if(source_position EQUAL -1)
    message(FATAL_ERROR "Selected graph target has no current artifact declaration")
endif()

# The repository's GCC presets select the default PC, not every custom App.
# Check their target against that configuration, not against the XT cutover.
if(PROJECT_CURRENT_PROFILE STREQUAL "default-pc-at-80386-1440k-hdd")
    string(FIND "${presets_source}" "\"targets\": [\"${current_target}\"]"
        preset_position)
    if(preset_position EQUAL -1)
        message(FATAL_ERROR "The current GCC preset does not name ${current_target}")
    endif()
endif()

message("M5:T197:S1:CURRENT-ARTIFACT-TARGET:${current_target}:OK")
