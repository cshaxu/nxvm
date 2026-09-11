if(NOT DEFINED LIBRARY_CMAKE)
    message(FATAL_ERROR "LIBRARY_CMAKE is required")
endif()

file(READ "${LIBRARY_CMAKE}" library_cmake)

if(library_cmake MATCHES "Curses")
    message(FATAL_ERROR "Linux shared-library configuration must not require Curses")
endif()

string(FIND "${library_cmake}" "find_package(Threads REQUIRED)" threads_find)
string(FIND "${library_cmake}"
    "target_link_libraries(host PUBLIC Threads::Threads)" host_sync_threads)
if(threads_find EQUAL -1 OR host_sync_threads EQUAL -1)
    message(FATAL_ERROR "Linux host must retain its explicit Threads linkage")
endif()

message(STATUS "Linux shared-library build contract verified")
