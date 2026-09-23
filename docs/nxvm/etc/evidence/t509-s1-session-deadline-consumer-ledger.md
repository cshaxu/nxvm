# T509 S1: Session Deadline-Consumer Ledger

## Frozen Universe And Rule

The audit covers all 33 C sources under `test/vm/` and `test/integration/`
that directly call `core_machine_run(session->core_machine, ...)`, plus the
one production VM runner. The rule is narrow: a runner that continues toward a
guest-visible checkpoint after `CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT` must
call the existing `vm_session_waiting_advance()` entry. That entry observes a
Core-published deadline and asks Core to advance; it is not a VM time writer.

| Disposition | Sources | Result |
| --- | --- | --- |
| Production owner | `src/vm/composition/session/runner.c` | Already uses the sole waiting entry point. |
| Existing compliant consumers | `vm_windows31_int13_trace_probe`, `vm_ata_pio_dos_smoke`, `vm_hdc_hdd_boot_smoke`, `vm_byob_dos_boot_probe`, `vm_fdc_read_track_dos_smoke`, `vm_rom_int13_hdd_type_smoke` | Already consume HLT/deadline before continuing. |
| Repaired long-running consumers | `vm_windows31_hdd_admission_probe`, `vm_cga_graphics_dos_smoke`, `vm_ega_planar_dos_smoke`, `vm_mouse_driver_dos_smoke`, `vm_dos_video_port_smoke`, `vm_model40_byob_retirement_capture`, `vm_rom_int15_memory_smoke` | Each now invokes the existing entry point when HLT occurs before its checkpoint. The EGA source builds two registered integration variants; both share that one repair. |
| Explicit HLT assertions | `vm_timer_firmware_smoke`, `vm_default_pc_at_apply_smoke`, `vm_model40_d4_compatibility_s25_smoke`, `vm_model40_rom_layout_s14_smoke` | The asserted result is HLT itself; advancing would invalidate the test subject. |
| Bounded non-continuation probes | `vm_cga_640_system_smoke`, `vm_cga_graphics_system_smoke`, `vm_ega_mode10_boot_smoke`, `vm_fault_outcome_runner_smoke`, `vm_fdc_dma_boundary_smoke`, `vm_fdc_t242_corpus_port_smoke`, `vm_ega_planar_system_smoke`, `vm_kbc_aux_guest_smoke`, `vm_model40_byob_s20_smoke`, `vm_model40_d4_skey_s23_smoke`, `vm_model40_fdc_s24_smoke`, `vm_model40_private_composition_s7_smoke`, `vm_pcat_composition_s4_smoke`, `vm_no_media_video_port_smoke`, `vm_rom_ega_int10_system_smoke`, `vm_rom_text_write_char_smoke` | Their named subject is a bounded instruction/port/reset/fault observation; no source continues toward an asynchronous device completion after HLT. |

Core-only tests are deliberately outside this universe: they verify direct Core
API semantics and must not acquire a VM scheduler. No session-runner source
locally advances ticks or calls `core_machine_advance_time()` as a substitute
for the waiting entry.

## Verification

The repaired long-running cohort passes: Model-40 retirement capture, ROM INT
15 memory, ROM INT 13 type-47, DOS video port, CGA DOS graphics, EGA DOS
graphics (including its ROM-INT10 variant), and mouse-driver DOS. The complete
repository unit and integration gates are required before T509 closure.
