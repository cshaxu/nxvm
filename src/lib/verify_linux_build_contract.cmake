if(NOT DEFINED LIBRARY_CMAKE)
    message(FATAL_ERROR "LIBRARY_CMAKE is required")
endif()

file(READ "${LIBRARY_CMAKE}" library_cmake)

if(library_cmake MATCHES "Curses")
    message(FATAL_ERROR "Linux shared-library configuration must not require Curses")
endif()

string(FIND "${library_cmake}" "find_package(Threads REQUIRED)" threads_find)
foreach(component IN ITEMS console host ui-base)
    string(FIND "${library_cmake}"
        "target_link_libraries(${component} PRIVATE Threads::Threads)" sync_threads)
    if(threads_find EQUAL -1 OR sync_threads EQUAL -1)
        message(FATAL_ERROR "Linux ${component} must declare its Threads linkage")
    endif()
endforeach()
if(library_cmake MATCHES "target_link_libraries\\(types[ \t\r\n]")
    message(FATAL_ERROR "types must not propagate implementation linkage")
endif()

message(STATUS "Linux shared-library build contract verified")
