add_library(mynes-app STATIC
    config.c
    command.c)
target_include_directories(mynes-app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mynes-app PUBLIC common-machine common-session mynes-core-protocol storage base)
mynes_enable_strict_warnings(mynes-app)

add_library(mynes-app-composition STATIC composition.c)
target_include_directories(mynes-app-composition PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mynes-app-composition PUBLIC mynes-app mynes-core-driver common-ui)
mynes_enable_strict_warnings(mynes-app-composition)

add_executable(mynes-0-0-0042 main.c)
set_target_properties(mynes-0-0-0042 PROPERTIES OUTPUT_NAME mynes_0_0_0042)
target_link_libraries(mynes-0-0-0042 PRIVATE mynes-app-composition)
if(MINGW)
    set(mynes_post_link_commands
        COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:mynes-0-0-0042>)
endif()
mynes_enable_strict_warnings(mynes-0-0-0042)
# assets/mynes owns the sole editable startup configuration.  Copy it beside
# a build-tree executable for local runs without creating a second template.
configure_file(${CMAKE_SOURCE_DIR}/assets/mynes/mynes.ini
    ${CMAKE_CURRENT_BINARY_DIR}/mynes.ini COPYONLY)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(mynes_artifact_arch x64)
    else()
        set(mynes_artifact_arch x86)
    endif()
    # Versioned runnable artifacts are a product deliverable.  ROMs remain
    # owner-local, but the paired executable and editable startup template are
    # intentionally retained beneath assets/mynes.
    set(mynes_artifact_directory ${CMAKE_SOURCE_DIR}/assets/mynes)
    set(mynes_artifact_name mynes_0_0_0042_${mynes_artifact_arch}.exe)
    # The executable reads the single editable template already retained beside
    # the versioned deliverable under assets/mynes.
    add_custom_command(TARGET mynes-0-0-0042 POST_BUILD
        ${mynes_post_link_commands}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${mynes_artifact_directory}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:mynes-0-0-0042>
            ${mynes_artifact_directory}/${mynes_artifact_name}
        VERBATIM)
endif()
