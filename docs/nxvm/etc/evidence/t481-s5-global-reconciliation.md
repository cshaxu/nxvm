# T481 S5 Global Reconciliation And Transfer

`M5:T481:S5:RECONCILIATION:OK`

| Audit family | Accepted evidence | Independent conclusion | Unique remaining receiver |
| --- | --- | --- | --- |
| CPU functions, delivery and retirement | T343, T449, T357--T363 and T481 S2 | One decoder/execution/delivery/retirement path; source-form clocks are L3 only where recorded. | T360/T388 source-context and timing ledgers; x87 and VME/PVI TODOs. |
| PIC, DMA and PIT | T456, T460, T461 and T481 S3 | One controller owner each; PIC has no wakeup deadline; qualified PIT may supply one. | Existing board-input/physical timing receivers. |
| RTC, KBC and 8272A FDC | T450 S10/S12/S14 and T481 S3 | One device/port/IRQ owner each; unqualified phase work blocks rather than fabricates a deadline. | Existing RTC, KBC and FDC phase-contract receivers. |
| VADP and HDC | T352/T480, T479 and T481 S3 | VADP owns video state/snapshot; HDC owns supported ATA, Compaq WD and IBM WD1003 personalities. | Existing video timing, IBM 5160/Xebec, ESDI and physical-media receivers. |
| Core, profile and VM pacing | T474, T475 and T481 S4 | Core alone writes guest time; profiles copy frozen inputs; VM only observes, waits in Standard, or skips that wait in Turbo. | Profile physical-timebase TODO; console-help correction below. |

## Verification And Review

- Documentation governance passed after each S and at this S5 review.
- The focused Core/VM CTest group passed 3/3: `core-machine-time-smoke`,
  `vm-session-initialization-atomicity-smoke`, and
  `vm-session-speed-policy-smoke`.
- Static source sweeps reconfirmed no VM/VDM call of Core's time publisher or
  generic advance API, and each controller's S3 disposition maps to the T474
  deadline composer or its explicit blocker path.
- A full `current-gates-gcc` attempt launched the configured two aggregate
  targets but made no observable progress and was stopped with its owned
  `ninja`/`ctest` children. It is not claimed as verification for this
  documentation-only audit; no source change requires a new runtime artifact.

## Ordered Transfers

1. Correct the stale Console `SPEED` help in the bounded next product-text
   repair; it must describe observation-based Standard pacing and Turbo
   no-wait, without adding a timing path.
2. The queued YAML cutover consumes the frozen resolver boundary, followed by
   the 5160 audit/device/phase sequence. That XT sequence is the sole receiver
   for the missing Xebec `320h`--`323h`, DMA3 and IRQ5 personality.
3. Existing timing and hardware TODOs retain CPU source gaps, x87, controller
   physical/service timing and profile physical-timebase evidence. They must
   refine their owner-local mechanisms, never create host tick injection or a
   parallel scheduler.

No duplicate state owner, hidden host-to-Core time injection, false selected
profile capability or unclassified frozen-universe row was found.

`M5:T481:S5:TRANSFERS:OK`

`M5:T481:S5:AUDIT-CLOSURE:OK`
