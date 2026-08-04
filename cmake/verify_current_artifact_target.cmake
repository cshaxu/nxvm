if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)
file(READ "${PROJECT_SOURCE_DIR}/CMakePresets.json" presets_source)

foreach(forbidden "add_vm_task_artifact" "add_vm_version_artifact")
    string(FIND "${cmake_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Historical artifact target helper remains: ${forbidden}")
    endif()
endforeach()

string(REGEX MATCHALL "add_current_vm_artifact\\(vm-[^ ]+" current_targets
    "${cmake_source}")
list(LENGTH current_targets current_target_count)
if(NOT current_target_count EQUAL 1)
    message(FATAL_ERROR
        "Expected exactly one current artifact target; found ${current_target_count}")
endif()
string(REGEX REPLACE "^add_current_vm_artifact\\((vm-[^ ]+)$" "\\1"
    current_target "${current_targets}")
if(current_target STREQUAL current_targets)
    message(FATAL_ERROR "Could not parse current artifact target: ${current_targets}")
endif()

string(FIND "${presets_source}" "\"targets\": [\"${current_target}\"]"
    preset_position)
if(preset_position EQUAL -1)
    message(FATAL_ERROR
        "The current GCC preset does not name ${current_target}")
endif()

message("M5:T197:S1:CURRENT-ARTIFACT-TARGET:${current_target}:OK")
