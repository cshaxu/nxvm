cmake_minimum_required(VERSION 3.23)
if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED POWERSHELL_EXECUTABLE OR
        NOT DEFINED WORK)
    message(FATAL_ERROR "Project source, PowerShell and work roots are required.")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/src/x86-nxvm/chips/probe"
    "${WORK}/src/app-probe/product")
file(WRITE "${WORK}/src/x86-nxvm/chips/probe/outer.c"
    "#include \"app-probe/product/binding_interface.h\"\n")
file(WRITE "${WORK}/src/x86-nxvm/chips/probe/reverse.c"
    "#include \"x86-nxvm/machine/machine_interface.h\"\n")
file(WRITE "${WORK}/src/app-probe/product/binding_interface.h" "\n")

execute_process(
    COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
        -File "${PROJECT_SOURCE_DIR}/tools/nxvm/Verify-DependencyDag.ps1"
        -RepositoryRoot "${WORK}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
file(REMOVE_RECURSE "${WORK}")
if(result EQUAL 0 OR NOT "${output}${error}" MATCHES
        "src/x86-nxvm/chips/probe/outer.c\\|app/probe" OR
        NOT "${output}${error}" MATCHES
        "src/x86-nxvm/chips/probe/reverse.c\\|x86-nxvm/machine")
    message(FATAL_ERROR
        "Dependency DAG probe did not reject outer and reverse edges: ${result}: ${output}${error}")
endif()
