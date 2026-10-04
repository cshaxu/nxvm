# S93 Final App Private-Board Reader Intake

This is a measured receiving inventory, not acceptance or permission to drop
assertions. All production board components are independently built; these
remaining App test/diagnostic imports still require owner reconciliation.
Rows are candidates based on external-scenario and product include evidence;
actual source/fixture review decides their final receiver. No new production
getter, moved external test, smaller assertion set or partial P closes a row.

| App source | Initial receiver | Private headers |
| --- | --- | --- |
| `test/app-nxvm/integration/model40/vm_model40_retirement_capture.c` | Received at App integration: two actual RTC reads use the existing Board CMOS fixture, two A20 reads and one execution-time physical read use the separately compiled Core fixture; original diagnostic body and criteria retained; dual-width diagnostic builds pass, not registered runtime proof | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/integration/windows/vm_windows31_setup_probe.c` | Received at App integration through the existing separately compiled Board controller fixture; original body and reports retained; explicit dual-width diagnostic builds pass, not a registered runtime pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/integration/windows/vm_windows31_hdd_admission_probe.c` | Received at App integration through the existing separately compiled Board controller fixture; original guest bytes, CHS/MBR/VBR checks and reports retained; explicit dual-width diagnostic builds pass, not a registered runtime pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/integration/hdd/vm_hdc_hdd_boot_smoke.c` | Received at App integration through the existing separately compiled Board controller fixture; original VBR/command assertions and reports retained; existing INI/overlay runtime passes once per width | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/integration/windows/vm_windows31_checkpoint.c` | Received at App integration through the existing separately compiled Board controller fixture; original checks/reports retained; diagnosed 6.42-second DIR completion exceeds old five-second command window, separate sixty-second directory containment preserves predicates; corrected runtime passes once per width (18.65s/19.70s), without production/INI change | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/integration/hdd/vm_ata_pio_dos_smoke.c` | Received at App integration; unused private fixture removed, chip constants use public HDC contract; complete body/guest program unchanged; existing INI/overlay runtime passes once per width | `x86/ibmpc-common/hdc.h` |
| `test/app-nxvm/integration/dos/vm_byob_dos_boot_probe.c` | Received at App integration through separately compiled Core/Board/AT/XT diagnostics; original 231 printf format strings remain in order; running-time reads, write-time CPU, FDC observer and controller fields retained; dual-width builds and unchanged default INI DOS-prompt probe pass, not whole-matrix proof | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-common/fdc.h`, `x86/ibmpc-at/kbc.h`, `x86/ibmpc-xt/xt_ppi_keyboard.h` |
| `test/app-nxvm/unit/core/devices/core_machine_cga_graphics_port_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_cga_graphics_port_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_cga_640_port_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_cga_640_port_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/profiles/ibm_5170_model_339/rom/ibm_5170_model_339_firmware_fdc_topology_smoke.c` | Received at App composition; existing Core port-presence and copied Board topology fixtures preserve the original firmware/Profile route assertions; full dual-width units and gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_compaq_cecg_s11_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_compaq_cecg_s11_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/integration/dos/vm_fdc_read_track_dos_smoke.c` | Received at App integration; unused private FDC/Core headers removed, entire scenario/guest bytes unchanged and existing external INI/overlay test passes once per width | `x86/ibmpc-common/fdc.h` |
| `test/app-nxvm/unit/core/devices/core_machine_compaq_cecg_s28_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_compaq_cecg_s28_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_controller_authority_smoke.c` | Received as Core attachment phases, Board FDC/HDC authority and Core binding identity with separately compiled Board fixture; original context, back-pointer and callback identity predicates retained; independent dual-width tests pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_compaq_cecg_s9_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_compaq_cecg_s9_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/machine/vm_cmos_rtc_port_smoke.c` | Received at App composition through public Core bus; original synthetic RTC/PIC time, interrupt, reset and diagnostic operations compile at Board owner; seed/Profile assertions retained and dual-width full units pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_ega_controller_port_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_ega_controller_port_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_enter_leave_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_enter_leave_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_ega_sequencer_port_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_ega_sequencer_port_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_ega_registration_transaction_smoke.c` | Received at Core registration receiver plus separately compiled Board video fixture; original allocation, collision, rollback and retry checks retained; independent and full dual-width tests pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_gpr_push_pop_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_gpr_push_pop_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_dma_rtc_authority_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_dma_rtc_authority_smoke.c`; independent dual-width tests pass; original refresh, IRQ, CMOS and reset checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_gpr_mov_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_gpr_mov_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_imul_immediate_s56_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_imul_immediate_s56_smoke.c`; independent dual-width 17-test/5-fixture batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_ega_planar_port_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_ega_planar_port_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_display_authority_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_display_authority_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_kbc_aux_port_smoke.c` | Received at `test/x86/ibmpc-at/core_machine_kbc_aux_port_smoke.c`; independent dual-width protocol tests pass through opaque Core; original checks retained | `x86/ibmpc-at/kbc.h` |
| `test/app-nxvm/unit/core/devices/core_machine_kbc_controller_smoke.c` | Received at AT protocol test plus separately compiled Core construction/reset and Board IRQ1 fixtures; original 172 checks and ten static data arrays preserved; independent and full dual-width tests pass | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-at/kbc.h` |
| `test/app-nxvm/unit/core/devices/core_machine_lea_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_lea_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_kbc_serial_cadence_smoke.c` | Received at `test/x86/ibmpc-at/core_machine_kbc_serial_cadence_smoke.c`; independent dual-width protocol tests pass through opaque Core; original checks retained | `x86/ibmpc-at/kbc.h` |
| `test/app-nxvm/unit/core/devices/core_machine_ega_external_port_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_ega_external_port_smoke.c`; independent dual-width ten-video batch passes; public Core bus and original hardware assertions | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/machine/vm_display_composition_s5_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_legacy_sreg_stack_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_legacy_sreg_stack_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_ega_sequencer_system_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_les_lds_s41_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_les_lds_s41_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_lss_lfs_lgs_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_lss_lfs_lgs_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_default_pc_at_apply_smoke.c` | Received at App composition with Core-owned synthetic-time fixture and Board-owned CMOS/config observations; original four media formats and 80186 refresh program retained; focused dual-width tests pass | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_fdc_port_smoke.c` | Received at App media composition through public Core bus and Board-owned exact FDC advance/refresh/PIC checks; original absent-media, change, format, write-protect, rate and data assertions retained; focused dual-width tests pass | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-common/fdc.h` |
| `test/app-nxvm/unit/core/devices/core_machine_les_lds_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_les_lds_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_fdc_authority_smoke.c` | Received at App composition with its original media registry check and all eight binding predicates compiled at Board; focused dual-width tests pass | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-common/fdc.h` |
| `test/app-nxvm/unit/core/devices/core_machine_moffs_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_moffs_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_hdc_port_smoke.c` | Received at App media/Profile composition with Board-owned HDC binding, exact service, copied phase and IRQ checks; original ATA multi-sector read/write and all error/NIEN/reset assertions retained; focused dual-width tests pass | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-common/hdc.h` |
| `test/app-nxvm/unit/core/machine/vm_fdc_t242_corpus_port_smoke.c` | Received at App synthetic-media composition through public Core bus/deadline, with Board-owned FDC IRQ/capture/advance operations; original 18-sector transfer, untouched RAM, motor and non-MFM cases retained; focused dual-width tests pass | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-common/fdc.h` |
| `test/app-nxvm/unit/core/devices/core_machine_pic_phase_s2_smoke.c` | Received at `test/x86/core/core_machine_pic_phase_s2_smoke.c`; real opaque PIC pair uses the public attachment contract, Core owns private transaction checks; all original failure expressions retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_planar_parity_nmi_s3_smoke.c` | Received at Shared Board parity receiver plus separately compiled Core parity-memory fixture; original real parity fault, Port-B/PIT/NMI, conflict and retry checks retained; independent and full dual-width tests pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_plan_smoke.c` | Received at test/x86/ibmpc-common/core_machine_plan_smoke.c with separately compiled Core assertions; all ten original case groups and fourteen markers retained, independent/full dual-width proof passes, no private peer layout or new production API | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_ibm_5170_model_339_cga_topology_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/machine_board_state.h`, `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/machine/vm_ibm_5170_model_339_composition_smoke.c` | Received at App composition; Core-owned timing/A20/registry predicates and Board-owned KBC/PIC/config checks preserve original 5170 firmware, memory, drive and CPU-retirement assertions; focused dual-width tests pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_port_assembly_smoke.c` | Received: Core route/allocation assertions in test/x86/core/port_assembly_fixture.c, private controller/topology checks in separately compiled Board fixture; App retains only D4 attachment and original driver. Complete original rollback/retry matrix passes independently and per width; no production API added | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_prefix_attributes_s64_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_prefix_attributes_s64_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_machine_initialization_atomicity_smoke.c` | Received at App initialization; copied plan scalars and Board-owned full timing/controller comparisons retain all original materialization checks, public Core getters still prove applied configuration; full dual-width units and gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_byob_s20_smoke.c` | Received at App composition; copied six-field Board plan observation and Core-local exact ROM-start/time checks preserve firmware ownership, reset, pacing and immutable config assertions; full dual-width units/gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_cecg_s11_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_cecg_s10_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_cecg_s13_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_cecg_s12_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/devices/core_machine_push_immediate_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_push_immediate_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_pusha_popa_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_pusha_popa_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_cecg_s9_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_cecg_s28_smoke.c` | Received at App composition through public Core/Board operations; original private route/aperture predicates compile separately at their real owner; dual-width full units pass | `x86/ibmpc-common/vadp.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_d4_compatibility_s25_smoke.c` | Received at App composition; original 19-tick PIT operation compiles at Board owner, D4 firmware/reset/Port B assertions remain App-owned and dual-width full units pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_fdc_s24_smoke.c` | Received at App Model40 composition with Board-owned controller fixtures and public Core bus; actual drive/config, PIO/DMA, IRQ/READY/reset and terminal-callback assertions retained; original dual-width case passes | `x86/ibmpc-common/fdc.h`, `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_dma_s17_smoke.c` | Received at App composition; Core-local wait/ready predicates, copied Board wiring, original duplicate-binding lifetime and pending/reset operations retain the full original checks; dual-width full units pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_hdc_s26_smoke.c` | Received at App composition; public Core bus and Board-owned HDC due-service/IRQ/actual connection values preserve the two drive-head reads, command/reset sequence and original memory media; dual-width full units pass | `x86/ibmpc-common/hdc.h`, `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_fdd_s18_smoke.c` | Received at App composition; Board-local fixture copies actual FDC connection config, not planned topology; original geometry/media/reset comparisons retained and full dual-width units and gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_integration_s8_smoke.c` | Received at App composition; Board FDC/HDC/KBC/RTC operations retain reset/IRQ/media checks; AT-owned AUX/scanning predicates restore original hardware checks instead of native-byte injection; full dual-width units/gates pass | `x86/ibmpc-common/fdc.h`, `x86/ibmpc-common/hdc.h`, `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model40_private_composition_s7_smoke.c` | Received at App composition; Core-local transaction values and original post-budget raw test I/O, copied Board clocks/plan and AT-owned AUX/scanning checks preserve Profile/D4/speaker/input assertions; full dual-width units/gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_ram_create_smoke.c` | Received at test/x86/core; Core owns allocation injection and RAM checks, compiled Board fixtures own projection/back-pointer; original allocation, null-output, clock preflight and publication predicates retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_pcat_composition_s4_smoke.c` | Received at App composition; Core-owned registry and original post-budget raw-write operation, Board-owned source/KBC/config checks; all original reset/timeline/NMI assertions retained; focused dual-width tests pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_model_339_clock_contract_smoke.c` | Received at App 5170 composition; copied actual Board clocks/timing and plan values replace layouts, original seven keyboard repeat assertions compile at AT owner with Board binding, serial complement retained; full dual-width units/gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_sreg_mov_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_sreg_mov_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_xchg_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_xchg_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_sign_extend_smoke.c` | Received at `test/x86/ibmpc-common/core_machine_sign_extend_smoke.c`; body unchanged; independent dual-width 15-case batch passes | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/core_machine_xt_ppi_keyboard_smoke.c` | Received at Board wiring/time test plus separately compiled XT state and Core route-allocation fixtures; all 148 original failure checks and FIFO bytes retained; independent and full dual-width tests pass | `x86/ibmpc-xt/xt_ppi_keyboard.h`, `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_timing_qualification_smoke.c` | Received at App four-Profile composition; public Core bus and separately compiled Board DMA request operation retain the original deadline assertions; full dual-width units and gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_two_session_isolation_smoke.c` | Received at App session composition; Core/Board identity predicates compile at their real owners, opaque FDC/HDC object identities restore the original allocation check, and public RAM/register/watchpoint checks remain; full dual-width units and gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/machine/vm_pcat_topology_s2_smoke.c` | Received at App composition; Core registry and Board source/KBC/config checks compile at their owners; original five routes, forbidden ports and pre-registration rejection retained; focused dual-width tests pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/fdc_boundary_negative.cmake` | Retained App integration gate: actual source review confirms App FDD/media and machine assembly checks alongside Shared controller checks; isolated source-text injections expose no private layout to compiled App code; original baseline and seven negatives pass once per width |  |
| `test/app-nxvm/unit/core/machine/vm_xt_5160_268_profile_smoke.c` | Received at App XT composition; actual FDC/HDC connection config, Core-local route predicates and public Core bus preserve the original complete Profile/route/BYOB assertions; full dual-width units and gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_bound_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_bound_board_smoke.c`; independent dual-width six-test batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_competition_80386_s1_smoke.c` | Received at test/x86/core with a separately compiled Board-owned DMA construction fixture; original HOLD, exact eight-tick advance and CPU/DMA/PIT/PIC trace predicates retained; independent dual-width tests pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_cmps_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_cmps_board_smoke.c`; independent dual-width six-test batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_cpu_pic_lifecycle_smoke.c` | Received at Core CPU-reset-identity and Board CPU/PIC-lifecycle tests, with one public guest-program fixture; independent dual-width tests pass; original identity, IRQ and reset assertions retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_d4_refresh_hold_smoke.c` | Received at App D4 composition with separately compiled Model40/Core/Board fixtures; actual refresh latch, callback, DMA wait, trace order and reset predicates retained; full dual-width units and gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_competition_s3_smoke.c` | Received at App D4 composition with separately compiled Core/Board fixtures; original DMA HOLD exclusion, ready/wait, exact tick and CPU/DMA/PIT/PIC trace predicates retained; full dual-width units and gates pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_explicit_time_s4_smoke.c` | Received at `test/x86/core/machine_explicit_time_s4_smoke.c`; Core owns private time/overflow checks, RTC observed through public bus; independent dual-width test passes with original checks | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_lods_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_lods_board_smoke.c`; independent dual-width 17-test/5-fixture batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_arpl_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_arpl_board_smoke.c`; independent dual-width six-test batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_movs_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_movs_board_smoke.c`; independent dual-width 17-test/5-fixture batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_port_io_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_port_io_board_smoke.c`; independent dual-width six-test batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_port_strings_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_port_strings_board_smoke.c`; independent dual-width six-test batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_reset_rom_alias_smoke.c` | Received at test/x86/core/machine_reset_rom_alias_smoke.c with separately compiled Board absent-memory predicate; all original reset aliases, mapping counts, failure injection, provider precedence and execution checks retained; independent/full dual-width tests pass | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_scheduler_smoke.c` | Received at `test/x86/core/machine_scheduler_smoke.c` and `test/x86/ibmpc-common/machine_board_timing_qualification_smoke.c`; independent dual-width tests pass; original Core callback and Board timing assertions retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_scas_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_scas_board_smoke.c`; independent dual-width 17-test/5-fixture batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_stos_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_stos_board_smoke.c`; independent dual-width 17-test/5-fixture batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_table_register_board_smoke.c` | Received at `test/x86/ibmpc-common/machine_table_register_board_smoke.c`; independent dual-width six-test batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/machine_rtc_storage_s4_smoke.c` | Received at `test/x86/ibmpc-common/machine_rtc_storage_s4_smoke.c`; independent dual-width trace/timeline tests pass; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/support/cpu_board_irq_fixture.h` | Received at `test/x86/ibmpc-common/cpu_board_irq_fixture.h`; independent dual-width 17-test/5-fixture batch passes; original checks retained | `x86/ibmpc-common/machine_board_state.h` |
| `test/app-nxvm/unit/core/devices/support/board_construction_fixture.h` | Received as Core allocation fixture and separately compiled Board construction fixture; original production projection/neutral allocation/attachment path retained; no source imports both private layouts | `x86/ibmpc-common/machine_board_state.h` |

All 99 original rows now have reconciled direct owner boundaries. This is
source-boundary disposition, not final whole-S or indirect-fixture proof.
App composition cases remain App-owned rather than becoming Shared scenarios.
Frozen-plan access and private construction operations
must also be counted even when no `board->` field occurs. Successful unit,
independent-build, gate, artifact, boot and integration results do not dispose
these source-boundary rows by implication.

## Additional Direct Core Readers In The Final Sweep

The 99-row Board inventory does not exhaust Core-private access. After the
CMOS/fault test cleanup, the direct include query finds these 19 C/header
paths. Each remains open for actual-use review; an include match alone is
neither proof of a live access nor permission to delete an assertion.
The two CMake negative-injection strings are excluded from this compiled-reader
list because they are deliberate forbidden-source specimens.

- `integration/dos/nxvm_default_profile_smoke.c`: reconciled; unused private
  memory include removed, complete scenario body unchanged, dual-width build proof.
- `integration/dos/vm_byob_dos_boot_probe.c`
- `unit/core/devices/core_machine_cpu_timing_preview_smoke.c`
- `unit/core/devices/core_machine_d4_memory_transaction_smoke.c`
- `unit/core/devices/core_machine_d4_platform_s4_smoke.c`: reconciled; real
  Core port/time/shutdown operations use separately compiled Core fixtures;
  original PIT byte-read effects, NMI/failsafe/speaker deadlines and halted
  shutdown-without-time-advance checks pass per width.
- `unit/core/devices/machine_80386_timing_manifest_runner.c`
- `unit/core/devices/machine_idt_privilege_pic_board_smoke.c`
- `unit/core/devices/machine_input_display_s5_smoke.c`: received at
  `test/x86/ibmpc-common/machine_input_display_s5_smoke.c`; actual Core guard
  setup uses a separately compiled Core fixture; all original guard/trace
  assertions pass independently and in root builds on both widths.
- `unit/core/devices/machine_outer_iret_pic_board_smoke.c`
- `unit/core/devices/machine_task_switch16_pic_board_smoke.c`
- `unit/core/devices/machine_time_smoke.c`: reconciled; Core fixture owns
  real timeline scheduling, Model40 fixture owns its refresh pending latch.
  Tick-1 versus tick-4 order, four invalid-axis conditions and instruction
  budget/reset assertions remain and pass per width.
- `unit/core/devices/machine_transaction_s2_smoke.c`
- `unit/core/devices/support/port_owner_fixture.h`
- `unit/core/devices/support/protected_pic_board_fixture.h`
- `unit/core/machine/ram_port_context_smoke.c`: received at
  `test/x86/core/ram_port_context_smoke.c`; original body/markers preserved,
  independent and root builds/tests pass on both widths.
- `unit/core/machine/vm_model40_d4_parity_s22_smoke.c`: reconciled; Core
  fixture flips the actual parity bit; original NMI, mask and clear checks
  pass per width, without mirrored parity state.
- `unit/core/machine/vm_model40_d4_skey_s23_smoke.c`: reconciled; Core fixture
  observes the actual A20 state; original D1 bytes, high ROM and reset-request
  checks pass per width.
- `unit/core/machine/vm_runner_error_propagation_smoke.c`: reconciled at App;
  actual lifecycle/provider fault injection belongs to the separately compiled
  Core fixture; existing paused exclusion and App ERROR assertions pass per width.
- `unit/core/machine/vm_x86_debug_mapping_smoke.c`: reconciled; unused Core
  layout include removed only. Complete App/Debug/lease assertions and marker
  pass per width through the existing public Core debug interface.

Paths above are relative to `test/app-nxvm/`. Query: `rg -n` for include
directives under `x86/{core,ibmpc-common,ibmpc-at,ibmpc-xt}` in App source/tests,
excluding `_interface.h`, explicit test fixture paths and CMake specimens,
then unique source-path projection. Relative/indirect fixture imports require
the separate dependency review and are not declared clear by this query.
The old DMA fixture still has its transaction-test caller; the runner-error
test still mutates actual firmware-provider/lifecycle state. Neither is closed
by the Board inventory's 98 reconciled rows.

The subsequent four-path reconciliation leaves 15 direct-private C/header
paths open. Runner fault injection now occurs at its Core owner; the earlier
sentence records the sweep's original finding, not current App layout access.
RAM/port and input/display receiver bodies retain all original predicates;
removal of their old App source/target definitions leaves one implementation
and one registered unit case for each. No unresolved row is disposed merely
because a header disappeared.

The next five dispositions initially left ten direct-private C/header paths
open. Subsequent D4 memory transaction and CPU/PIC fixture reconciliation
reduced the actual include count to five. CPU timing preview now has a Shared
Board receiver and separately compiled Core publication/preview fixture;
its independent dual-width regression passes. Transaction S2 now compiles at
the Shared Core test owner. DMA registers directly on that fixture's actual
Core port table; the obsolete borrowed-port copy helpers are deleted. Original
reset cancellation, memory cycle failure, ordering and provenance checks pass
independently and in root builds on both widths. The 80386 manifest's actual
FPU completion and transaction opcode/ModRM/kind checks now compile at the
Core test owner, retaining its nonzero/at-most-19 tick bound. Root dual-width
execution passes. The BYOB probe subsequently receives its private observations
through separately compiled Core/Board/AT/XT test owners. The actual direct
private import/member-access query is empty; indirect fixture/recipe review
and complete verification remain open.
Indirect private dependencies remain a separate
mandatory sweep. Board intake has 99 directly reconciled rows.
Full acceptance is not claimed.

The additional 18-path CPU-private App sweep is reconciled by actual use:
thirteen public-contract/encoding cases drop obsolete private includes;
execution-context and five CPU/PIC receivers (with shared protected-PIC
support) move to test/x86/chips/cpu. Their Core port fixture is separately
compiled, not textually imported. All nineteen root cases pass on both
widths; six received cases pass in independent builds on both widths.
The direct App chip-private include query is empty. Timing-recipe and remaining
indirect dependencies still require review before the complete acceptance gate.
