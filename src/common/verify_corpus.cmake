# The Common corpus is independently buildable only against the public Lib
# surface.  Keep this check structural: product and Core meaning belongs in
# the adapters outside this directory, not in Common.
if(NOT DEFINED COMMON_ROOT)
    message(FATAL_ERROR "COMMON_ROOT is required")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT:PATH=${COMMON_ROOT}
        -DCORPUS_NAME:STRING=Common
        -P "${COMMON_ROOT}/verify_manifest.cmake"
    COMMAND_ERROR_IS_FATAL ANY)

set(common_required_files
    CMakeLists.txt
    machine/machine.c machine/machine_interface.h
    session/session.c session/session_interface.h
    ui/console_host.c ui/console_host.h ui/presentation.c ui/ui_interface.h
    xasm32/aasm32.c xasm32/aasm32.h xasm32/dasm32.c
    xasm32/dasm32.h xasm32/xasm32.c xasm32/xasm32.h
    xasm32/xasm32_interface.h
    debug/command.c debug/command.h debug/command_runtime.h
    debug/debug.c debug/debug_interface.h)
foreach(common_file IN LISTS common_required_files)
    if(NOT EXISTS "${COMMON_ROOT}/${common_file}")
        message(FATAL_ERROR "Common corpus omits required file: ${common_file}")
    endif()
endforeach()

file(GLOB_RECURSE common_corpus_files RELATIVE "${COMMON_ROOT}"
    "${COMMON_ROOT}/*.c" "${COMMON_ROOT}/*.h" "${COMMON_ROOT}/CMakeLists.txt")
foreach(common_file IN LISTS common_corpus_files)
    file(READ "${COMMON_ROOT}/${common_file}" common_text)
    foreach(common_forbidden
            "#include \"core/"
            "#include \"vm/"
            "#include \"type.h\""
            "#include <windows.h>"
            "#include <X11/")
        string(FIND "${common_text}" "${common_forbidden}" common_forbidden_at)
        if(NOT common_forbidden_at EQUAL -1)
            message(FATAL_ERROR
                "Common corpus has a forbidden product/native include in ${common_file}: ${common_forbidden}")
        endif()
    endforeach()
endforeach()

message(STATUS "Common standalone Lib-only corpus: OK")
