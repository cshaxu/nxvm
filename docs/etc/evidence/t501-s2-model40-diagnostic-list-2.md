# T501 S2: Model-40 Firmware-Visible Diagnostic List 2

`T501-S2-MODEL40-DIAGNOSTIC-LIST-2`

This list maps every T501 S2 List-1 relation to the current sole owner.  The
only nonterminal replay symptom is classified; none is silently converted into
a product defect or a success claim.

| List-1 ID | Current owner and route | Current observation | Disposition |
| --- | --- | --- | --- |
| D1 | `vm_session_model40_materialize_controllers()` constructs two fitted 1.2-MB drive bindings, with `VM_SESSION_MEDIA_FDD_ID` only on A:. | Both selects are present; B: has no media binding. | Conforms to the selected External-L2 construction fact without duplicating media state. |
| D2 | Firmware text reaches the VADP-owned copied display snapshot. | Release replay renders `601` and `RESUME`. | Conforms as diagnostic publication; cause remains D11. |
| D3 | `src/core/machine/fdc.c` owns command/result phases, ready sampling, media-change latching, DMA and IRQ. | Replay reaches `Sense Drive Status`; it has no FDC transfer terminal. | Controller remains live; the missing board relation is not repaired here. |
| D4 | `core_machine_kbc` owns scan-set-2 input and the selected VM route owns host input. | S1's F1 injection is not a pre-diagnostic escape and cannot clear the displayed state. | Conforms; no keyboard repair. |
| D5 | `core_machine_fdc_execute()` forms ST3 from selected unit/head, media capability, `core_machine_fdc_drive_ready()`, and cylinder. | The ROM's selected-unit status and later read path proceed through the two-fitted-drive, one-media topology. | Current chip behavior remains one owner; exact board READY timing remains D11. |
| D6 | `core_machine_media_registry` is bound once to the Core plan; FDC observes it through its configured drive bindings. | The selected image is mounted only as A:. | Conforms; no parallel media cache. |
| D7 | `vm_profile_model40_external_rom_is_valid()` validates the external selection; `core_machine_d4_memory_config` supplies the immutable split mapping. | ROM reaches the FDC diagnostic, rather than a reset/mapping fault. | Conforms; no ROM-side workaround. |
| D8 | `core_machine` owns event/deadline progression; T499's one seam serves FDC/DMA/PIC and all current scheduler clients. | Continuous progress reaches the selected FDC command; there is no HLT/no-deadline stall. | Conforms; no scheduler reopening. |
| D9 | Existing Core KBC queue and `vm_session` input route. | No evidence of lost F1 before the ROM makes it a continuation action. | Conforms; not the earliest owner. |
| D10 | `core_machine_vadp` owns state and copied snapshot; Win32 presentation only consumes it. | `601`/`RESUME` is visible in the copied frame. | Conforms; no second display state. |
| D11 | No owner can be selected honestly for physical board READY/change timing, but drive count is no longer part of that ambiguity. | The former `601` path was removed by the selected External-L2 fitted-drive construction; the fresh replay reaches a DOS prompt through normal FDC/DMA/IRQ/KBC paths. | Physical board timing remains source-gated; no guessed READY/timing value was added. |
| D12 | No source-qualified board timing owner. | Current 8272A timing plan remains its own existing contract; no DeskPro-specific drive signal value was derived. | **Transfer with D11.** |

## Similar-Issue Sweep

The selected source and code inspection found no separate current defect in
ROM mapping, D4 memory, Core scheduler progression, controller IRQ/DMA
ownership, media ownership, keyboard delivery, or copied display publication.
Those are distinct owners with direct observations above.  Treating `601` as
proof of any one of them would be an inference from a symptom and would violate
the task boundary.

## S5 Repair Disposition

The complete repair batch is construction-only: the frozen profile supplies
two fitted 1.2-MB drive slots, but binds media only to A:.  Core FDC remains the
sole owner of select, status, DMA, IRQ and media observation.  D12 and the
physical part of D11 remain bounded source-gated debt.
