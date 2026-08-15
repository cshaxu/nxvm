# T375 S19: Model-339 FDC Service-Time Crosswalk

## Decision

The 1.44 MB compatibility route is now concretely bounded as an aftermarket
**TEAC FD-235HF-A540** on the selected Model-339 8272A-compatible path.  It is
not an IBM factory configuration, firmware fact, or replacement-controller
identity.  No external source, ROM, media, trace, binary, or implementation
text is imported.

The TEAC specification supplies usable drive-local limits: 500 kbit/s at
1.44 MB, 300 RPM, nominal 3 ms track-to-track, spindle start no greater than
480 ms, and 100 ms average rotational latency.  The accepted Model-339
virtual-time source publishes nominal 8 MHz ticks, so the following values are
valid conversion inputs, rather than retirement-callback counts:

| Drive-local quantity | Primary value | 8 MHz conversion | Permitted use |
| --- | --- | --- | --- |
| One data byte at 500 kbit/s | 16 us | 128 ticks | Per-byte DRQ/DMA service gate. |
| 512-byte payload at 500 kbit/s | 8.192 ms | 65,536 ticks | Payload-only lower bound; it excludes gaps and sector search. |
| One revolution at 300 RPM | 200 ms | 1,600,000 ticks | Explicit spindle/index phase model. |
| Mean rotational latency | 100 ms | 800,000 ticks | Validation range/checkpoint, not a fixed first-sector delay. |
| Track-to-track movement | nominal 3 ms | 24,000 ticks | Seek/recalibrate step input after per-drive head-position state exists. |
| Spin-up | no greater than 480 ms | no greater than 3,840,000 ticks | Motor-ready upper boundary only; not a fabricated exact completion. |

The primary source is TEAC's [FD-235HF-A529/A540/A591 Specification Rev. B](https://ftpmirror.your.org/pub/misc/bitsavers/pdf/teac/FD-235HFA5XX_Specification_Rev_B.pdf).
The NEC uPD765A/Intel 8272A material already retained by T366/T374 remains the
authority for command phases, `Specify` SRT/HLT/HUT fields, DRQ and IRQ
semantics.  It does not select an IBM Type-3 board delay or turn one value
above into a controller command latency.

`M5:T375:S19:FDC-SERVICE-CROSSWALK:OK`

## Current-Owner Sweep

| Current route | Actual implementation | S19 disposition |
| --- | --- | --- |
| Command acceptance | The final command byte changes `fdc.c` to `PENDING_COMMAND`; the next readiness tick executes it. | Replace callback-count completion with one elapsed-time deadline or explicitly source-gated immediate route. Do not use generic 256 us. |
| Reset and Ready attention | DOR reset clears state immediately; `refresh` observes Ready edges and raises the accepted attention IRQ6 route. | The TEAC spin-up maximum applies only after motor modelling is added. Preserve current reset/attention functional behavior. |
| Seek/recalibrate | Execution currently assigns the destination cylinder and raises IRQ6 in the next service step. | Add per-drive physical head position plus `abs(delta) * 24,000`-tick nominal step scheduling in a later S; separately preserve the 480-ms maximum as a bound, not a completion scalar. |
| Read/write payload | DMA or non-DMA transfer consumes/provides bytes immediately whenever the current execution phase is entered. | Add a 128-tick byte gate for 500-kbit/s media and make each DREQ/DACK byte wait for it. The 65,536-tick payload lower bound must be observable. |
| Sector availability | The raw IMG provider exposes logical sectors but neither index position nor encoded gaps. | Add no synthetic sector-search delay in this S. A later scheduler must choose an explicit deterministic index-phase contract or retain the reference-exhausted search interval. |
| Completion and IRQ6 | After the last byte, current code changes to `PENDING_COMPLETE`; the next readiness tick publishes result/IRQ6. | Completion must follow the final byte gate; PIC selection/INTA timing remains a separate board receiver. |
| DMA2 handoff | The FDC asserts an opaque DMA2 request for an execution phase; DMA owns grant/transfer/terminal behavior. | Do not introduce a parallel DMA engine. The later FDC scheduler only controls when a byte request is made eligible. |

## Secondary 86Box Cross-Check

Read-only inspection used 86Box v6.0 revision `4fef696a`,
`src/floppy/fdc.c`; nothing was copied.  Its generic `fdc_at` implementation
uses a timer and distinguishes reset (8 us), invalid-command completion (100
us), and a generic 256-us callback for seek/default command paths, while its
read/write path begins controller/media processing without using that generic
delay.  It also derives 500-kbit/s operation and delegates media/rotation to
its drive layer.

That behavior confirms the important shape--controller state and drive/media
availability are separate, and one readiness tick is not an adequate service
model.  Its 8/100/256-us values are implementation choices of a generic AT
controller, not primary facts for a Type-3 Model-339 or the named aftermarket
drive.  They are therefore **not** NXVM timing constants.

## Next Receiver

The next bounded implementation S may add elapsed-time state at the existing
`core_machine_fdc` owner: controller deadline, per-drive head/motor state and
per-byte 500-kbit/s gate, consumed from the already-published machine elapsed
clock.  It must preserve the single DMA2 and IRQ6 owners, cancel pending work
on DOR reset/finalize/media loss, and prove no transfer byte or completion IRQ
appears before its gated deadline.  It must not claim sector-search timing,
exact spin-up completion, FDC-controller oscillator conversion, electrical
waveforms, or final 5170 L3.

The separately queued raw-IMG sidecar task remains the only receiver for
Deleted Data, Control Mark and Scan fidelity.  This crosswalk does not expand
ordinary IMG media semantics.

## Review And Verification

The review inspected all pending-command, pending-completion, transfer,
format, seek/recalibrate, Ready, reset/finalize, DMA request, IRQ6 and
readiness-timeline routes in `fdc.c` and `machine.c`, plus the HDC deferred
service contrast.  The external reference inspection stayed read-only outside
the repository.  Documentation governance passed after adding this indexed
evidence; no executable surface changed in S19 P1.
