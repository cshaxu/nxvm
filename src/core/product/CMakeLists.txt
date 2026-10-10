cmake_minimum_required(VERSION 3.23)

get_filename_component(CORE_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
get_filename_component(CORE_SOURCE_ROOT "${CORE_ROOT}/.." ABSOLUTE)
if(NOT TARGET core-machine)
    add_subdirectory("${CORE_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/core")
endif()
if(NOT TARGET x86-product)
    add_subdirectory("${CORE_SOURCE_ROOT}/product" "${CMAKE_CURRENT_BINARY_DIR}/product")
endif()

add_library(core-product STATIC factory.c ini.c startup.c)
target_include_directories(core-product PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-product PUBLIC core-machine x86-product
    types storage base)
