if(EXISTS "${CMAKE_SOURCE_DIR}/src/app-nxvm/profiles/xt")
    message(FATAL_ERROR "My5160 retains an obsolete peer App source owner")
endif()
file(GLOB_RECURSE my5160_sources "${CMAKE_SOURCE_DIR}/src/app-my5160/*.c"
    "${CMAKE_SOURCE_DIR}/src/app-my5160/*.h")
foreach(source IN LISTS my5160_sources)
    file(READ "${source}" text)
    if(text MATCHES "#[ \t]*include[^\n]*app-(nxvm|my5170|mydeskpro386)/")
        message(FATAL_ERROR "My5160 includes a peer App: ${source}")
    endif()
endforeach()
if(NXVM_PRODUCT_MACHINE_KEY STREQUAL "xt" AND
        (NOT NXVM_PRODUCT_ARTIFACT_ROOT STREQUAL "${CMAKE_SOURCE_DIR}/assets/my5160" OR
         NOT NXVM_PRODUCT_ARTIFACT_DIRECTORY STREQUAL NXVM_PRODUCT_ARTIFACT_ROOT))
    message(FATAL_ERROR "My5160 uses another App artifact root")
endif()
message(STATUS "My5160 independent App boundary: OK")
