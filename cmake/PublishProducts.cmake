# Repository composition owns publishing; shared packages remain independent.
function(repository_publish_product app target filename)
    if(NOT WIN32 OR NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        return()
    endif()
    add_custom_target(publish-${app}
        COMMAND "${CMAKE_COMMAND}" "-DINPUT=$<TARGET_FILE:${target}>"
            "-DOUTPUT=${CMAKE_SOURCE_DIR}/assets/${app}/${filename}"
            -P "${CMAKE_SOURCE_DIR}/cmake/PublishProduct.cmake"
        DEPENDS ${target} VERBATIM)
endfunction()
