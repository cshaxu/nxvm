if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" project_cmake)

foreach(forbidden "VM_RUNTIME_SOURCES")
    string(FIND "${project_cmake}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T447 build ownership retains forbidden ${forbidden}")
    endif()
endforeach()

string(REGEX MATCH
    "target_link_libraries\\(vm-composition PUBLIC[^\\)]*(user32|gdi32|vm-platform)"
    composition_native "${project_cmake}")
if(composition_native)
    message(FATAL_ERROR "T447 vm-composition must not propagate a host-native dependency")
endif()

string(REGEX MATCH
    "if\\(WIN32\\)[ \t\r\n]+target_link_libraries\\(vm-platform PUBLIC user32 gdi32\\)"
    native_owner "${project_cmake}")
if(NOT native_owner)
    message(FATAL_ERROR "T447 requires vm-platform to own Win32 GUI libraries")
endif()

string(REGEX MATCH
    "add_executable\\(\\$\\{target\\} EXCLUDE_FROM_ALL[ \t\r\n]+src/vm/main\\.c\\)"
    artifact_source "${project_cmake}")
if(NOT artifact_source)
    message(FATAL_ERROR "T447 artifact must compile only vm/main.c directly")
endif()

message("M5:T447:S1:BUILD-OWNERSHIP:OK")
