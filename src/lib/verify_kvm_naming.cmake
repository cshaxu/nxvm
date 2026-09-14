if(NOT DEFINED KVM_NAMING_ROOT)
    message(FATAL_ERROR "KVM naming root is required")
endif()

string(CONCAT retired_prefix "u" "i")
string(TOUPPER "${retired_prefix}" retired_prefix_upper)
string(CONCAT retired_class "Lib" "U" "x" "Window")

if(EXISTS "${KVM_NAMING_ROOT}/src/lib")
    set(naming_roots
        "${KVM_NAMING_ROOT}/CMakeLists.txt"
        "${KVM_NAMING_ROOT}/src/app"
        "${KVM_NAMING_ROOT}/src/lib"
        "${KVM_NAMING_ROOT}/test/integration"
        "${KVM_NAMING_ROOT}/test/support"
        "${KVM_NAMING_ROOT}/test/unit"
        "${KVM_NAMING_ROOT}/test/lib"
        "${KVM_NAMING_ROOT}/test/common"
        "${KVM_NAMING_ROOT}/docs/design"
        "${KVM_NAMING_ROOT}/docs/proposals"
        "${KVM_NAMING_ROOT}/docs/states/CURRENT.md")
else()
    set(naming_roots "${KVM_NAMING_ROOT}")
endif()

foreach(naming_root IN LISTS naming_roots)
    if(IS_DIRECTORY "${naming_root}")
        file(GLOB_RECURSE naming_files LIST_DIRECTORIES FALSE
            "${naming_root}/*.c" "${naming_root}/*.h" "${naming_root}/*.cmake"
            "${naming_root}/*.md" "${naming_root}/CMakeLists.txt")
    elseif(EXISTS "${naming_root}")
        set(naming_files "${naming_root}")
    else()
        continue()
    endif()
    foreach(naming_file IN LISTS naming_files)
        file(RELATIVE_PATH naming_path "${KVM_NAMING_ROOT}" "${naming_file}")
        string(REPLACE "\\" "/" naming_path "${naming_path}")
        if(naming_path MATCHES "^common/ui/" OR naming_path MATCHES "^test/lib/fixtures/")
            continue()
        endif()
        if(naming_path MATCHES "(^|/)${retired_prefix}-" OR
           naming_path MATCHES "(^|/)${retired_prefix}_")
            message(FATAL_ERROR "Obsolete KVM spelling in path: ${naming_path}")
        endif()
        file(READ "${naming_file}" naming_text)
        # common/ui is intentionally the product-independent interaction
        # owner.  Its canonical public include is not the retired lib UI
        # component taxonomy this verifier rejects.
        string(REPLACE "common/ui/ui_interface.h" "" naming_text "${naming_text}")
        if(naming_text MATCHES "(^|[^A-Za-z0-9_])${retired_prefix}_" OR
           naming_text MATCHES "(^|[^A-Za-z0-9_])${retired_prefix_upper}_" OR
           naming_text MATCHES "(^|[^A-Za-z0-9_])${retired_class}([^A-Za-z0-9_]|$)")
            message(FATAL_ERROR "Obsolete KVM spelling in: ${naming_path}")
        endif()
    endforeach()
endforeach()
