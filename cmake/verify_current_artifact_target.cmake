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
if(NOT current_targets STREQUAL "add_current_vm_artifact(vm-0-5-0205")
    message(FATAL_ERROR "Unexpected current artifact target: ${current_targets}")
endif()

string(FIND "${presets_source}" "\"targets\": [\"vm-0-5-0205\"]"
    preset_position)
if(preset_position EQUAL -1)
    message(FATAL_ERROR "The current GCC preset does not name vm-0-5-0205")
endif()

message("M5:T197:S1:CURRENT-ARTIFACT-TARGET:OK")
