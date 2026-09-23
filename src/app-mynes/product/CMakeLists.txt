add_library(mynes-app STATIC
    config.c
    command.c)
target_include_directories(mynes-app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mynes-app PUBLIC common-machine common-session mynes-core-protocol storage)
mynes_enable_strict_warnings(mynes-app)

add_library(mynes-app-composition STATIC composition.c)
target_include_directories(mynes-app-composition PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mynes-app-composition PUBLIC mynes-app mynes-core-driver common-ui)
mynes_enable_strict_warnings(mynes-app-composition)

add_executable(mynes-0-1-0011 main.c)
set_target_properties(mynes-0-1-0011 PROPERTIES OUTPUT_NAME mynes_0_1_0011)
target_link_libraries(mynes-0-1-0011 PRIVATE mynes-app-composition)
if(MINGW)
    target_link_options(mynes-0-1-0011 PRIVATE -s)
endif()
mynes_enable_strict_warnings(mynes-0-1-0011)
# assets/binary-mynes owns the sole editable startup configuration.  Copy it beside
# a build-tree executable for local runs without creating a second template.
configure_file(${CMAKE_SOURCE_DIR}/assets/binary-mynes/mynes.ini
    ${CMAKE_CURRENT_BINARY_DIR}/mynes.ini COPYONLY)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(mynes_artifact_arch x64)
    else()
        set(mynes_artifact_arch x86)
    endif()
    # Versioned runnable artifacts are a product deliverable.  ROMs remain
    # owner-local, but the paired executable and editable startup template are
    # intentionally retained beneath assets/binary-mynes.
    set(mynes_artifact_directory ${CMAKE_SOURCE_DIR}/assets/binary-mynes)
    set(mynes_artifact_name mynes_0_1_0011_${mynes_artifact_arch}.exe)
    # The executable reads the single editable template already retained beside
    # the versioned deliverable under assets/binary-mynes.
    add_custom_command(TARGET mynes-0-1-0011 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${mynes_artifact_directory}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:mynes-0-1-0011>
            ${mynes_artifact_directory}/${mynes_artifact_name}
        VERBATIM)
endif()
