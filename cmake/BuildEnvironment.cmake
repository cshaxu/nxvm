include_guard(GLOBAL)

# Repository policy is injected by its root or its standalone build tool,
# never by an independently reusable source/test corpus.
get_filename_component(repository_root "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)
if(CMAKE_GENERATOR MATCHES "^Ninja")
    if(IS_SYMLINK "${repository_root}/build" OR
       IS_SYMLINK "${repository_root}/build/.ninja")
        message(FATAL_ERROR "Ninja cache roots must not be links.")
    endif()
    set(ninja_root "${repository_root}/build/.ninja")
    file(REAL_PATH "${CMAKE_BINARY_DIR}" actual_binary)
    cmake_path(IS_PREFIX ninja_root "${actual_binary}" NORMALIZE inside_ninja)
    if(NOT inside_ninja OR actual_binary STREQUAL ninja_root)
        message(FATAL_ERROR "Ninja build directories must be below ${ninja_root}")
    endif()
endif()

option(REPOSITORY_USE_CCACHE "Use optional repository-local compiler cache" OFF)
if(REPOSITORY_USE_CCACHE)
    find_program(repository_ccache NAMES ccache REQUIRED)
    set(cache_root "${repository_root}/build/.ccache")
    if(IS_SYMLINK "${repository_root}/build" OR IS_SYMLINK "${cache_root}" OR
       IS_SYMLINK "${cache_root}/tmp")
        message(FATAL_ERROR "Compiler cache roots must not be links.")
    endif()
    file(MAKE_DIRECTORY "${cache_root}/tmp")
    set(CMAKE_C_COMPILER_LAUNCHER
        "${CMAKE_COMMAND};-E;env;CCACHE_DIR=${cache_root};CCACHE_TEMPDIR=${cache_root}/tmp;CCACHE_MAXSIZE=5G;${repository_ccache}"
        CACHE STRING "Repository compiler cache route" FORCE)
else()
    set(CMAKE_C_COMPILER_LAUNCHER "" CACHE STRING "Compiler cache disabled" FORCE)
endif()
