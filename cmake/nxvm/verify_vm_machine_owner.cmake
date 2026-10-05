if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_text)
file(GLOB old_machine_sources "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/*.[ch]")
if(old_machine_sources)
    message(FATAL_ERROR "obsolete App Machine implementation remains")
endif()
file(GLOB_RECURSE shared_machine_sources
    "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/*.h")
foreach(source IN LISTS shared_machine_sources)
    file(READ "${source}" shared_source)
    if(shared_source MATCHES "#[ \t]*include[ \t]*[\"]app-")
        message(FATAL_ERROR "Shared Machine includes App source: ${source}")
    endif()
endforeach()
if(cmake_text MATCHES "vm-composition" OR
        cmake_text MATCHES "src/(vm|app-nxvm/machine)/composition/session")
    message(FATAL_ERROR "obsolete VM composition executor route remains in CMake")
endif()
if(EXISTS "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/composition/session")
    message(FATAL_ERROR "obsolete VM composition executor source root remains")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/machine_interface.h" event_header)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/product/composition.c" app_source)

# Common owns the sole lifecycle queue and worker.  App composition constructs
# it from the vm/machine driver, then forwards copied facts to Common Session.
foreach(required IN ITEMS
    "common_machine_start"
    "common_machine_pause"
    "common_machine_reset"
    "common_machine_resume"
    "common_machine_stop")
    string(FIND "${lifecycle_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "VM machine Common lifecycle route lacks ${required}")
    endif()
endforeach()

foreach(required IN ITEMS
    "app->factory.prepare"
    "common_machine_create"
    "app->factory.bind")
    string(FIND "${app_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "VM app Common composition lacks ${required}")
    endif()
endforeach()

string(FIND "${lifecycle_source}" "common_machine_create" position)
if(NOT position EQUAL -1)
    message(FATAL_ERROR "core/core still constructs Common")
endif()

foreach(required IN ITEMS
    "common_machine_set_state_sink"
    "common_machine_set_frame_sink"
    "common_session_enqueue_runtime_completed"
    "common_session_enqueue_frame_completed")
    string(FIND "${app_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "VM app does not forward Common fact ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "vm_machine_result" "vm_machine_set_result_sink"
    "vm_machine_publish_result" "HWND" "HANDLE")
    string(FIND "${event_header}${lifecycle_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "obsolete VM result or native-handle route remains: ${forbidden}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "set_lifecycle_reporter" "set_display_reporter")
    string(FIND "${lifecycle_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "obsolete split result callback remains: ${forbidden}")
    endif()
endforeach()

file(GLOB_RECURSE app_sources
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/product/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/product/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/product/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/product/*.h")
foreach(source IN LISTS app_sources)
    file(RELATIVE_PATH relative "${PROJECT_SOURCE_DIR}/src/app-nxvm/product" "${source}")
    file(READ "${source}" source_text)
    if(source_text MATCHES "core_machine_(create|destroy|reset|run|request_stop|capture_display_snapshot)")
        message(FATAL_ERROR "direct Core runtime access outside core/machine: ${relative}")
    endif()
endforeach()

# A fixed product has one constructor supplied by its build. Cross-profile
# fixture selection must not recreate a production plan registry or union.
file(GLOB_RECURSE profile_plans
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/machine_plan.[ch]"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/*/machine_plan.c"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/profiles/machine_plan.c"
    "${PROJECT_SOURCE_DIR}/src/app-my5170/profiles/machine_plan.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/machine_plan.c")
foreach(source IN LISTS profile_plans)
    file(READ "${source}" plan_text)
    if(plan_text MATCHES "union[ \t\r\n]*\\{|VM_PROFILE_MACHINE_PLAN_(DEFAULT|IBM|XT|MODEL40)|config->profile_kind")
        message(FATAL_ERROR "Fixed composition retains model dispatch/storage: ${source}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/product/machine_binding.c" factory_text)
if(NOT factory_text MATCHES "\\.prepare = VM_PROFILE_PLAN_CREATE" OR
        factory_text MATCHES "config->profile_kind|vm_profile_machine_plan_create\\(")
    message(FATAL_ERROR "Machine factory does not use its fixed build constructor")
endif()
file(READ "${PROJECT_SOURCE_DIR}/cmake/nxvm/NxvmProductProfile.cmake" profile_build)
file(READ "${PROJECT_SOURCE_DIR}/src/app-my5160/CMakeLists.txt" xt_build)
file(READ "${PROJECT_SOURCE_DIR}/src/app-my5170/CMakeLists.txt" at_build)
file(READ "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/CMakeLists.txt" model40_build)
string(APPEND profile_build "\n${xt_build}\n${at_build}\n${model40_build}")
foreach(constructor IN ITEMS default 5170 xt model40)
    if(NOT profile_build MATCHES "set\\(NXVM_PROFILE_PLAN_CREATE vm_profile_machine_plan_create_${constructor}\\)")
        message(FATAL_ERROR "Build does not bind the ${constructor} constructor")
    endif()
endforeach()
file(GLOB_RECURSE model40_sources
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/*.h")
foreach(source IN LISTS model40_sources)
    file(READ "${source}" model40_text)
    if(model40_text MATCHES "app-nxvm/profiles/default_profile/|vm_profile_ibm_5170_(values|plan)_create")
        message(FATAL_ERROR "Model40 depends on another AT model's construction: ${source}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/input_interface.h" input_contract)
if(input_contract MATCHES "profile_kind|VM_MACHINE_PROFILE_(DEFAULT|IBM|COMPAQ)|app-nxvm/")
    message(FATAL_ERROR "Machine value contract retains identity or model-specific observations")
endif()

foreach(retired IN ITEMS machine_plan.c machine_plan.h machine_plan_interface.h selection_interface.h)
    if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/${retired}")
        message(FATAL_ERROR "Generic App construction mechanism remains: ${retired}")
    endif()
endforeach()
file(GLOB_RECURSE app_profile_sources
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/profiles/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-my5170/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-my5170/profiles/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/*.h")
foreach(source IN LISTS app_profile_sources)
    file(READ "${source}" source_text)
    if(source_text MATCHES "vm_profile_model40_rom_materialize|image\\[index \\* 2u\\]|even_bytes\\[logical >> 1u\\]")
        message(FATAL_ERROR "App retains duplicate ROM byte interleave: ${source}")
    endif()
    if(source_text MATCHES "vm_profile_machine_plan_(validate|publish|destroy|describe|core_config_get|timing_rules_get|topology_get|firmware_provider_get|firmware_context_get|materialize\\(|hdc_present|external_firmware)|vm_profile_name\\(")
        message(FATAL_ERROR "Retired shared App plan path remains: ${source}")
    endif()
endforeach()
if(NOT profile_build MATCHES "set\\(NXVM_PROFILE_MONITOR_NAME \"ibm-5160-model-268\"\\)")
    message(FATAL_ERROR "XT fixed Product identity is not preserved")
endif()
if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/product/config.c" OR
        EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/machine_factory.c" OR
        EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/machine_factory_interface.h" OR
        factory_text MATCHES "vm_machine_create|vm_machine_runtime_config|vm_app_read_(information|speed)")
    message(FATAL_ERROR "App retains shared factory/Product adaptation")
endif()

message(STATUS "M5 NXVM machine Common-owner and copied-fact boundary verified")
