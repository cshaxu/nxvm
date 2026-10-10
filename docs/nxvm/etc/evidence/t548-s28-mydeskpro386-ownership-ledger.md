# T548 S28 — MyDeskPro386 Unit Ownership Ledger

## Scope and disposition

All 25 current Model 40 unit source entries were compared with Core's chip,
board and machine receivers.  Core retains generic CECG, DMA, FDC, HDC, PIT,
Port B, memory transaction and execution matrices.  The following App entries
remain because each selects a Model 40-specific D4 attachment, CMOS/firmware
identity, fixed geometry, or composed wiring context that Core intentionally
does not select.

| Entry group | Entries | Canonical owner and reason | Disposition |
| --- | --- | --- | --- |
| D4 component contract | `core_machine_d4_platform`, `d4_memory_transaction`, `d4_port_b_assembly`, `d4_prefetch_locality`, `d4_refresh_deadline`, `machine_d4_refresh_hold` | MyDeskPro386 owns D4 memory and Port B/refresh attachment; the Core receivers prove the generic transaction and scheduler mechanisms. | Retain App tests. |
| Model 40 plan/media/ROM | `model40_dma_deadline`, `model40_floppy_geometry`, `model40_plan`, `rom/model40_rom_layout`, `vm_model40_byob_boot_media`, `vm_model40_byob`, `vm_model40_cmos_seed`, `vm_model40_fdc`, `vm_model40_hdc`, `vm_model40_machine_integration` | Fixed CMOS seed, ROM chip layout, 1.2 MB floppy/HDC geometry, BYOB policy and composed media route are Model 40 facts. | Retain App tests. |
| CECG chosen environment | `vm_model40_cecg`, `cecg_cpu_video_gate`, `cecg_feature_environment`, `cecg_input_status`, `cecg_io_base`, `cecg_odd_even` | Core proves CECG mechanism.  These entries prove the Model 40 D4/CECG selected address map and CPU-visible route. | Retain App tests. |
| D4 composed system path | `vm_model40_d4_a20_reset`, `d4_compatibility`, `d4_map`, `d4_parity`, `vm_model40_dma`, `vm_model40_composition` | A20, D4 compatibility/map/parity, DMA and complete construction are Model 40 composition facts. | Retain App tests. |

There is no equal-or-stronger Core receiver for any listed selected-profile
assertion.  Earlier S24 already separated the generic Core paths from these
Model 40 increments; S28 does not revive a duplicate generic test.

## Mechanical boundary repair

Twenty-five Model 40 test sources used the host C runtime directly.  They now
include `lib/types/file.h` and use `lib_c_printf` / `lib_c_fprintf` with
`lib_c_stderr`.  This preserves every diagnostic string, assertion, target and
registration; it only restores the repository C-runtime vocabulary boundary.

## Verification

The full 28-route `app-mydeskpro386` unit label passes on both widths:

- x64: 28/28, 25.00 s real time;
- x86: 28/28, 3.05 s real time.

No production source, firmware, media, INI, snapshot, CTest registration or
deployed executable input changed.  Desktop and external integration were not
run by this S.
