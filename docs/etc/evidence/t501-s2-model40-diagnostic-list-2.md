# T501 S2: Model-40 Firmware-Visible Diagnostic List 2

`T501-S2-MODEL40-DIAGNOSTIC-LIST-2`

This list maps every T501 S2 List-1 relation to the current sole owner.  The
only nonterminal replay symptom is classified; none is silently converted into
a product defect or a success claim.

| List-1 ID | Current owner and route | Current observation | Disposition |
| --- | --- | --- | --- |
| D1 | `vm_session_model40_materialize_controllers()` constructs one FDD binding, `VM_SESSION_MEDIA_FDD_ID`, and three invalid bindings. | Exactly one media-backed drive exists. | Conforms.  Do not add a second binding. |
| D2 | Firmware text reaches the VADP-owned copied display snapshot. | Release replay renders `601` and `RESUME`. | Conforms as diagnostic publication; cause remains D11. |
| D3 | `src/core/machine/fdc.c` owns command/result phases, ready sampling, media-change latching, DMA and IRQ. | Replay reaches `Sense Drive Status`; it has no FDC transfer terminal. | Controller remains live; the missing board relation is not repaired here. |
| D4 | `core_machine_kbc` owns scan-set-2 input and the selected VM route owns host input. | S1's F1 injection is not a pre-diagnostic escape and cannot clear the displayed state. | Conforms; no keyboard repair. |
| D5 | `core_machine_fdc_execute()` forms ST3 from selected unit/head, media capability, `core_machine_fdc_drive_ready()`, and cylinder. | The ROM's final observed command selects unit 1 and receives `11h`: selected unit, track 0, not ready. | Current chip behavior is consistent with the one-mounted-drive topology.  Exact ROM-board expectation is D11. |
| D6 | `core_machine_media_registry` is bound once to the Core plan; FDC observes it through its configured drive bindings. | The selected image is mounted only as A:. | Conforms; no parallel media cache. |
| D7 | `vm_profile_model40_external_rom_is_valid()` validates the external selection; `core_machine_d4_memory_config` supplies the immutable split mapping. | ROM reaches the FDC diagnostic, rather than a reset/mapping fault. | Conforms; no ROM-side workaround. |
| D8 | `core_machine` owns event/deadline progression; T499's one seam serves FDC/DMA/PIC and all current scheduler clients. | Continuous progress reaches the selected FDC command; there is no HLT/no-deadline stall. | Conforms; no scheduler reopening. |
| D9 | Existing Core KBC queue and `vm_session` input route. | No evidence of lost F1 before the ROM makes it a continuation action. | Conforms; not the earliest owner. |
| D10 | `core_machine_vadp` owns state and copied snapshot; Win32 presentation only consumes it. | `601`/`RESUME` is visible in the copied frame. | Conforms; no second display state. |
| D11 | No owner can be selected honestly: current FDC sees a legal unready unit, while the available original sources do not define the ROM's intended unit-1/result relation. | The ROM reports `601` after the observed status path. | **Transfer as one source-gated Model-40 FDD/ROM diagnostic relation.** Do not alter FDC readiness, CMOS, FDD count, ROM, input, or scheduler. |
| D12 | No source-qualified board timing owner. | Current 8272A timing plan remains its own existing contract; no DeskPro-specific drive signal value was derived. | **Transfer with D11.** |

## Similar-Issue Sweep

The selected source and code inspection found no separate current defect in
ROM mapping, D4 memory, Core scheduler progression, controller IRQ/DMA
ownership, media ownership, keyboard delivery, or copied display publication.
Those are distinct owners with direct observations above.  Treating `601` as
proof of any one of them would be an inference from a symptom and would violate
the task boundary.

## Implementation Batch

Empty by design.  A repair must wait for a source that relates the exact Model
40 ROM diagnostic expectation to a real board/FDC/drive signal.  This is a
finite transfer, not an abandoned ambiguity: its receiver is the D11--D12
relation named in List 1 and its [bounded debt entry](../../states/TODO.md#hardware-and-compatibility-debt).
