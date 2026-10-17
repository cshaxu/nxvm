# Machine-owned registration; included by the package root.
add_test(NAME x86-core.machine/chip-boundary COMMAND "${CMAKE_COMMAND}"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_boundary.cmake")
set_tests_properties(x86-core.machine/chip-boundary PROPERTIES
    LABELS "unit;x86-core" TIMEOUT 30)
add_library(x86-core-machine-prefetch-scheduler-test OBJECT
    "${X86_CORE_ROOT}/machine/machine_scheduler.c")
target_link_libraries(x86-core-machine-prefetch-scheduler-test PRIVATE x86-core-machine)
target_compile_definitions(x86-core-machine-prefetch-scheduler-test PRIVATE
    x86_core_cpu_execution_advance_prefetch_reservation=test_cpu_prefetch_grant
    x86_core_cpu_execution_advance_bus=test_cpu_bus_service)
add_executable(x86-core-machine-prefetch-locality-smoke ${CMAKE_CURRENT_LIST_DIR}/machine_prefetch_locality_smoke.c)
target_link_libraries(x86-core-machine-prefetch-locality-smoke PRIVATE
    x86-core-machine-prefetch-scheduler-test x86-core-machine)
add_test(NAME x86-core.machine/prefetch-locality-smoke COMMAND x86-core-machine-prefetch-locality-smoke)
set_tests_properties(x86-core.machine/prefetch-locality-smoke PROPERTIES LABELS "unit;x86-core" TIMEOUT 30)
list(APPEND SHARED_X86_CORE_TEST_TARGETS x86-core-machine-prefetch-locality-smoke)

add_executable(x86-core-machine-vm86-iret-smoke ${CMAKE_CURRENT_LIST_DIR}/machine_vm86_iret_smoke.c)
target_link_libraries(x86-core-machine-vm86-iret-smoke PRIVATE x86-core-machine)
add_test(NAME x86-core.machine/vm86-iret-smoke COMMAND x86-core-machine-vm86-iret-smoke)
set_tests_properties(x86-core.machine/vm86-iret-smoke PROPERTIES LABELS "unit;x86-core" TIMEOUT 30)
list(APPEND SHARED_X86_CORE_TEST_TARGETS x86-core-machine-vm86-iret-smoke)
add_executable(x86-core-machine-protected-iret-smoke ${CMAKE_CURRENT_LIST_DIR}/machine_protected_iret_smoke.c)
target_link_libraries(x86-core-machine-protected-iret-smoke PRIVATE x86-core-machine)
add_test(NAME x86-core.machine/protected-iret-smoke COMMAND x86-core-machine-protected-iret-smoke)
set_tests_properties(x86-core.machine/protected-iret-smoke PROPERTIES LABELS "unit;x86-core" TIMEOUT 30)
list(APPEND SHARED_X86_CORE_TEST_TARGETS x86-core-machine-protected-iret-smoke)
foreach(core_case IN ITEMS call-gate tss-iomap-port-authorization task-switch-cross-width
        80286-instruction-timing-ledger)
    string(REPLACE "-" "_" core_file "${core_case}")
    set(core_target "x86-core-machine-${core_case}-smoke")
    add_executable(${core_target} "${CMAKE_CURRENT_LIST_DIR}/machine_${core_file}_smoke.c")
    target_link_libraries(${core_target} PRIVATE x86-core-machine)
    add_test(NAME "x86-core.machine/${core_case}-smoke" COMMAND ${core_target})
    set_tests_properties("x86-core.machine/${core_case}-smoke" PROPERTIES LABELS "unit;x86-core" TIMEOUT 30)
    list(APPEND SHARED_X86_CORE_TEST_TARGETS ${core_target})
endforeach()
add_executable(x86-core-machine-neutral-link-smoke ${CMAKE_CURRENT_LIST_DIR}/neutral_link.c)
target_link_libraries(x86-core-machine-neutral-link-smoke PRIVATE x86-core-machine)
add_test(NAME x86-core.machine/neutral-link-smoke COMMAND x86-core-machine-neutral-link-smoke)
set_tests_properties(x86-core.machine/neutral-link-smoke PROPERTIES LABELS "unit;x86-core" TIMEOUT 30)
list(APPEND SHARED_X86_CORE_TEST_TARGETS x86-core-machine-neutral-link-smoke)

foreach(core_case IN ITEMS
    "x86-core-machine-ram-port-context-smoke|ram_port_context_smoke.c|x86-core-machine"
    "x86-core-machine-port-registration-smoke|machine_port_registration_smoke.c|x86-core-machine"
    "x86-core-machine-attachment-phases-smoke|machine_attachment_phases_smoke.c|x86-core-machine"
    "x86-core-machine-real-mode-386-address-smoke|real_mode_386_address_smoke.c|x86-core-machine"
    "x86-core-machine-real-mode-386-rep-cmps-smoke|real_mode_386_rep_cmps_smoke.c|x86-core-machine"
    "x86-core-machine-real-mode-corpus-smoke|real_mode_corpus_smoke.c|x86-core-machine"
    "x86-core-machine-real-mode-tick-smoke|real_mode_tick_smoke.c|x86-core-machine"
    "x86-core-machine-int-ivt-smoke|cpu_int_ivt_smoke.c|x86-core-machine"
    "x86-core-machine-configuration-smoke|machine_configuration_smoke.c|x86-core-machine"
    "x86-core-machine-memory-device-registration-smoke|machine_memory_device_registration_smoke.c|x86-core-machine"
    "x86-core-machine-retirement-observation-smoke|machine_retirement_observation_smoke.c|x86-core-machine"
    "x86-core-machine-fpu-8087-smoke|machine_fpu_8087_smoke.c|x86-core-machine"
    "x86-core-machine-transaction-lifecycle-smoke|machine_transaction_lifecycle_smoke.c|x86-core-machine-observable"
    "x86-core-machine-transaction-smoke|machine_transaction_smoke.c|x86-core-machine-observable"
    "x86-core-machine-instruction-timing-ledger-smoke|machine_instruction_timing_ledger_smoke.c|x86-core-machine"
    "x86-core-machine-scheduler-smoke|machine_scheduler_smoke.c|x86-core-machine"
    "x86-core-machine-cpu-timing-preview-smoke|machine_cpu_timing_preview_smoke.c|x86-core-machine-observable"
    "x86-core-machine-rom-route-transaction-smoke|machine_rom_route_transaction_smoke.c|x86-core-machine"
    "x86-core-machine-immutable-rom-mapping-smoke|machine_immutable_rom_mapping_smoke.c|x86-core-machine"
    "x86-core-machine-rational-clock-smoke|machine_rational_clock_smoke.c|x86-core-machine"
    "x86-core-machine-executor-run-smoke|executor_run_smoke.c|x86-core-machine"
    "x86-core-machine-memory-inspection-smoke|memory_inspection_smoke.c|x86-core-machine"
    "x86-core-machine-80386-protected-io-timing-smoke|machine_80386_protected_io_timing_smoke.c|x86-core-machine"
    "x86-core-machine-string-io-timing-smoke|machine_string_io_timing_smoke.c|x86-core-machine"
    "x86-core-machine-80386-privileged-timing-smoke|machine_80386_privileged_timing_smoke.c|x86-core-machine"
    "x86-core-machine-8086-instruction-timing-ledger-smoke|machine_8086_instruction_timing_ledger_smoke.c|x86-core-machine"
    "x86-core-machine-80186-instruction-timing-ledger-smoke|machine_80186_instruction_timing_ledger_smoke.c|x86-core-machine"
    "x86-core-machine-80386-secondary-integer-timing-smoke|machine_80386_secondary_integer_timing_smoke.c|x86-core-machine"
    "x86-core-machine-cpu-profile-gate-smoke|machine_cpu_profile_gate_smoke.c|x86-core-machine"
    "x86-core-machine-instruction-timing-smoke|machine_instruction_timing_smoke.c|x86-core-machine"
    "x86-core-machine-descriptor-system-smoke|descriptor_system_smoke.c|x86-core-machine"
    "x86-core-machine-80386-paging-smoke|80386_paging_smoke.c|x86-core-machine-observable"
    "x86-core-machine-contract-smoke|core_contract_smoke.c|x86-core-machine"
    "x86-core-machine-lifecycle-smoke|machine_lifecycle_smoke.c|x86-core-machine"
    "x86-core-machine-stopped-lifecycle-smoke|machine_stopped_lifecycle_smoke.c|x86-core-machine"
    "x86-core-machine-memory-reconfigure-smoke|machine_memory_reconfigure_smoke.c|x86-core-machine"
    "x86-core-machine-trace-smoke|machine_trace_smoke.c|x86-core-machine-observable"
    "x86-core-machine-external-time-trace-smoke|machine_external_time_trace_smoke.c|x86-core-machine-observable"
    "x86-core-machine-debug-smoke|machine_debug_smoke.c|x86-core-machine"
    "x86-core-machine-non-pc-board-smoke|non_pc_board_smoke.c|x86-core-machine"
)
    string(REPLACE "|" ";" core_fields "${core_case}")
    list(GET core_fields 0 core_target)
    list(GET core_fields 1 core_source)
    list(GET core_fields 2 core_library)
    string(REGEX REPLACE "^x86-core-machine-" "" core_test "${core_target}")
    add_executable(${core_target} "${CMAKE_CURRENT_LIST_DIR}/${core_source}")
    target_link_libraries(${core_target} PRIVATE ${core_library})
    add_test(NAME "x86-core.machine/${core_test}" COMMAND ${core_target})
    set_tests_properties("x86-core.machine/${core_test}" PROPERTIES LABELS "unit;x86-core" TIMEOUT 30)
    list(APPEND SHARED_X86_CORE_TEST_TARGETS ${core_target})
endforeach()

shared_register_test(x86-core machine/system_power x86-core-machine)
shared_register_test(x86-core machine/timeline x86-core-machine)
add_executable(x86-core-machine-fpu-interface-smoke ${CMAKE_CURRENT_LIST_DIR}/machine_fpu_interface_smoke.c)
target_link_libraries(x86-core-machine-fpu-interface-smoke PRIVATE x86-core-machine)
add_test(NAME x86-core.machine/fpu-interface-smoke COMMAND x86-core-machine-fpu-interface-smoke)
set_tests_properties(x86-core.machine/fpu-interface-smoke PROPERTIES LABELS "unit;x86-core" TIMEOUT 30)
list(APPEND SHARED_X86_CORE_TEST_TARGETS x86-core-machine-fpu-interface-smoke)
add_executable(x86-core-machine-entry-plan-smoke ${CMAKE_CURRENT_LIST_DIR}/machine_entry_plan_smoke.c)
target_link_libraries(x86-core-machine-entry-plan-smoke PRIVATE x86-core-machine)
add_test(NAME x86-core.machine/entry-plan-smoke COMMAND x86-core-machine-entry-plan-smoke)
set_tests_properties(x86-core.machine/entry-plan-smoke PROPERTIES LABELS "unit;x86-core" TIMEOUT 30)
list(APPEND SHARED_X86_CORE_TEST_TARGETS x86-core-machine-entry-plan-smoke)
