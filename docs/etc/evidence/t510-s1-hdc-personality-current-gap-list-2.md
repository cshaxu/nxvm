# T510 S1 HDC Personality Current-Gap List 2

`M5:T510:S1:HDC-LIST-2:OK`

## One Owner Trace

`VM frozen profile/composition -> copied core_machine_hdc_config ->
core_machine_hdc -> media provider plus PIC/DMA binding -> Core scheduler`

`src/core/machine/hdc.c` owns task-file state, Xebec state, DRQ, IRQ source,
pending phase and deadline.  `src/vm` owns only the construction-time copied
configuration.  The media provider owns inserted bytes and persistence.  The
scheduler consumes `core_machine_hdc_next_due_tick()`; no VM timer, firmware
delay, second CHS state or media cache participates.

| Row | List-1 relation | Current path | Gap and one-batch disposition |
| --- | --- | --- | --- |
| A1--A2 | ATA PIO protocol and Other-L2 quantum | default-PC/AT config supplies `service_ticks = 200`; `core_machine_hdc_capture_command()` schedules `next_service_tick`; `core_machine_hdc_advance_elapsed()` completes it. | Retain.  The focused VM ATA test proves the non-immediate deadline path. |
| W1--W3 | WD1003 logical protocol and distinct L2 command/sector relation | Model-339 supplies one `service_ticks = 16000`; task-file state and `3F6h` handling are one Core path. | Repair: the one scalar is reused for both command and inter-sector transition, so it cannot represent the separately identified 2-ms/980-us Other-L2 relations. |
| C1--C3 | Compaq port/IRQ/PIO semantics and unavailable service duration | Model-40 construction supplies explicit Compaq protocol and one Core state; `3F7h` is composed at the Core port boundary. | Repair: the unqualified hard-coded 16000 duration is currently indistinguishable from a sourced value.  Remove it rather than relabel an arbitrary duration as L2. |
| X1--X3 | Xebec DCB/DMA/IRQ and Other-L2 command visibility | XT profile selects Xebec, but `core_machine_xebec_complete_dcb()` immediately enters DMA/response and `core_machine_hdc_next_due_tick()` rejects the idle task-file phase. | Repair: add one Xebec pending-command phase using the existing HDC elapsed/deadline field; profile supplies the frozen 250-step Other-L2 quantum. |
| all | Reset/cancel/result/IRQ ownership | HDC reset/finalize clear its own IRQ; status/result reads acknowledge the same source; media remains external. | Retain.  No timing relation is invented for reset, raw MFM/ECC/seek, physical ready/index or firmware ROM behavior. |

## Coherent Implementation Batch

1. Replace the single task-file `service_ticks` with two generic, profile-free
   Core inputs: command completion and next-sector completion.  This deletes
   the false assertion that all controller phases share one duration.
2. Give the Xebec branch one pending DCB-completion phase which reuses the
   existing HDC elapsed tick, deadline query and advance path.  It must not
   create an XT scheduler, VM callback or duplicate timer.
3. Configure only source-qualified external-L2 inputs in VM: ATA remains its
   200-step model; Model-339 receives 16000/7840; XT receives 250/0 because
   only DCB completion has an observable timer phase.  Model-40 supplies zero
   service values pending its own qualified model.
4. Extend the owner-local HDC tests and the three profile tests to prove that
   each supported nonzero relation publishes and consumes the Core deadline,
   while the Compaq zero relation remains explicitly immediate/non-qualified.

The batch is additive only where it removes a false shared interpretation: it
keeps exactly one HDC state object, one scheduler receiver, one media boundary
and one construction-time profile route.
