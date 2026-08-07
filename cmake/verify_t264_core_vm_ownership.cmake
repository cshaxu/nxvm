if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE core_sources
    "${PROJECT_SOURCE_DIR}/src/core/*.c"
    "${PROJECT_SOURCE_DIR}/src/core/*.h")
foreach(source IN LISTS core_sources)
    file(READ "${source}" source_text)
    string(REGEX MATCH "#include[ \t]*[\"<]vm/" core_depends_on_vm
        "${source_text}")
    if(core_depends_on_vm)
        message(FATAL_ERROR "T264 core source depends on VM: ${source}")
    endif()
endforeach()

set(rtc "${PROJECT_SOURCE_DIR}/src/core/machine/rtc.c")
set(qdcga "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/qdcga.c")
set(machine_devices "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c")
set(cmake_source "${PROJECT_SOURCE_DIR}/CMakeLists.txt")
foreach(source IN ITEMS "${rtc}" "${qdcga}" "${machine_devices}" "${cmake_source}")
    if(NOT EXISTS "${source}")
        message(FATAL_ERROR "T264 ownership source missing: ${source}")
    endif()
    file(READ "${source}" source_text)
    set("source_${source}" "${source_text}")
endforeach()

file(READ "${rtc}" rtc_source)
file(READ "${machine_devices}" machine_devices_source)
foreach(forbidden IN ITEMS "t_cpu" "flagMaskNMI" "set_nmi_mask")
    string(FIND "${rtc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T264 core RTC retains VM/CPU policy: ${forbidden}")
    endif()
endforeach()
foreach(required IN ITEMS "core_machine_rtc_advance"
    "core_machine_pic_irq_source_assert" "core_machine_pic_irq_source_deassert")
    string(FIND "${rtc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T264 core RTC contract is incomplete: ${required}")
    endif()
endforeach()
foreach(required IN ITEMS "core_machine_set_nmi_mask"
    "core_machine_install_port_provider" "core_machine_rtc_select_register")
    string(FIND "${machine_devices_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T264 VM RTC adapter is incomplete: ${required}")
    endif()
endforeach()
string(FIND "${machine_devices_source}" "->rtc.registers" position)
if(NOT position EQUAL -1)
    message(FATAL_ERROR "T264 VM RTC adapter mirrors core state")
endif()

file(READ "${qdcga}" qdcga_source)
foreach(forbidden IN ITEMS "profile_binding_set_video"
    "profile_binding_configure_text_video" "profile_binding_memory")
    string(FIND "${qdcga_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T264 QDCGA retains direct VADP shortcut: ${forbidden}")
    endif()
endforeach()
string(FIND "${qdcga_source}" "core_machine_profile_binding_write_port" position)
if(position EQUAL -1)
    message(FATAL_ERROR "T264 QDCGA does not use the core port boundary")
endif()

foreach(removed IN ITEMS
    "src/core/machine/firmware.c"
    "src/core/machine/firmware_interface.h"
    "src/vm/profile/default_profile/firmware/default_profile.c"
    "src/vm/profile/default_profile/firmware/default_profile.h")
    if(EXISTS "${PROJECT_SOURCE_DIR}/${removed}")
        message(FATAL_ERROR "T264 obsolete firmware registry remains: ${removed}")
    endif()
endforeach()
file(READ "${cmake_source}" cmake_text)
foreach(forbidden IN ITEMS "core-machine-firmware" "vm-firmware-smoke"
    "vm-default-profile-firmware-smoke")
    string(FIND "${cmake_text}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T264 CMake retains obsolete firmware target: ${forbidden}")
    endif()
endforeach()

file(GLOB_RECURSE vm_sources
    "${PROJECT_SOURCE_DIR}/src/vm/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/*.h")
foreach(source IN LISTS vm_sources)
    file(READ "${source}" source_text)
    string(FIND "${source_text}" "core_machine_run(" run_position)
    if(NOT run_position EQUAL -1 AND
        NOT source STREQUAL "${PROJECT_SOURCE_DIR}/src/vm/composition/session/runner.c")
        message(FATAL_ERROR "T264 VM-side CPU execution path: ${source}")
    endif()
endforeach()

message("M5:T264:S3:CORE-VM-PCAT-OWNERSHIP:OK")
