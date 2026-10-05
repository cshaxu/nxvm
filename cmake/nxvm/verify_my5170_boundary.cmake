file(GLOB_RECURSE my5170_sources "${CMAKE_SOURCE_DIR}/src/app-my5170/*.c"
    "${CMAKE_SOURCE_DIR}/src/app-my5170/*.h")
foreach(source IN LISTS my5170_sources)
    file(READ "${source}" text)
    if(text MATCHES "#[ \t]*include[^\n]*app-(nxvm|my5160|mydeskpro386)/")
        message(FATAL_ERROR "My5170 includes a peer App: ${source}")
    endif()
endforeach()
if(NXVM_PRODUCT_MACHINE_KEY STREQUAL "at" AND
        (NOT NXVM_PRODUCT_ARTIFACT_ROOT STREQUAL "${CMAKE_SOURCE_DIR}/assets/my5170" OR
         NOT NXVM_PRODUCT_ARTIFACT_DIRECTORY STREQUAL NXVM_PRODUCT_ARTIFACT_ROOT))
    message(FATAL_ERROR "My5170 uses another App artifact root")
endif()
message(STATUS "My5170 independent App boundary: OK")
