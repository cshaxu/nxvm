if(NOT DEFINED PROJECT_BUILD_TYPE OR
   NOT PROJECT_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR
        "Product artifacts may be published only from Release.")
endif()
