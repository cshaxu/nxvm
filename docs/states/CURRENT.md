# Project Status

## Current Work

**Active: M5 T375 S20.**

| Task | Compact progress |
| --- | --- |
| T375 S19 | Accepted P1 `cb3ebb93`: names the aftermarket TEAC FD-235HF-A540 timing inputs, retains 86Box as a non-authoritative cross-check, and transfers elapsed-time FDC scheduling to the next S. [Evidence](../etc/evidence/t375-s19-fdc-service-time-crosswalk.md). |

## M5 T375 S20 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved continuation toward full L3, selected the Model-339 aftermarket 1.44 MB route, and authorized secondary reference use. S19 accepts the TEAC FD-235HF-A540 500-kbit/s conversion input. |
| Objective | Add FDC-owner elapsed-time gating for 500-kbit/s DMA data bytes so DRQ/DMA2 eligibility recurs no faster than 128 Model-339 machine ticks and completion IRQ6 follows the final gated byte. |
| Non-goals | No unqualified command, seek, rotational, motor, reset or controller delay; no physical waveform, FDC controller clock conversion, raw-IMG metadata work, guest media, ROM, generic DMA rewrite, or Model-339 L3 claim. |
| Reference Baseline | T375 S19 crosswalk: 500 kbit/s equals 16 us or 128 nominal 8-MHz ticks per byte. `machine.c` already publishes explicit virtual time and owns timeline order; `fdc.c` owns DMA request lifecycle, transfer byte and IRQ6 source. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | `src/core/machine/fdc.[ch]`, `src/core/machine/machine.c`, focused FDC timing smoke and CMake registration, Current/evidence/index. The existing FDC owner gains an internal timestamped advance operation; no public test-only input API or duplicate DMA route. |
| Applicable Rules | Execution lifecycle, architecture single-owner/one DMA transaction path, coding owner-local helper and no test-only public API, source policy boundary, and documentation/index rules. |
| Verification | Build/run the focused FDC timing smoke, retained FDC and DMA smokes, and documentation governance. Prove first DMA byte is eligible, no second byte before 128 ticks, the next byte becomes eligible at its gate, DOR reset cancels a pending gate, and completion IRQ6 does not precede the final byte. |
| Expected Markers | `M5:T375:S20:FDC-DMA-CADENCE:OK`, retained FDC/DMA markers, and `Documentation governance checks passed`. |
| Asset Needs | No external runtime asset. S19's indexed TEAC primary source is a documentation reference only. |
| Reporting Requirements | Report exact code ownership, focused results, push P1, review result, and all remaining FDC mechanical/command-time transfers. |
| Stop Conditions | Stop if enforcing a byte gate requires a second DMA owner, a public test-only ingress, a source-incompatible rate, or an unbounded host-time path. |
| Exit Criteria | Production FDC code uses the existing machine-time/timeline owner to gate each DMA byte; reset/media cancellation clears it; focused proof covers cadence and IRQ ordering; no unsupported delay is represented as an exact 5170 fact. |
| Original Owner Request | Continue implementing L3; when authority lacks deterministic timing, use 86Box/MAME/PCjs only as bounded secondary references. Keep the 1.44 MB configuration a non-factory field upgrade. |
| Similar-Issue Sweep | Sweep DMA read/write/terminal, non-DMA reads/writes, format, read-track, DOR reset, drive-not-ready, FDC reset/finalize, DMA reset and all machine callers. Apply the gate only to the source-backed 500-kbit/s DMA byte path; transfer other modes explicitly. |

## Current Technical Baseline

- **Current developer artifact:** T369 S4 `vm-0-5-0369` /
  `build/output/nxvm_0_5_0369.exe`; its SHA-256 and source commit are
  recorded in the T369 S4 closure audit.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, immutable ROM
  mapping, and validated real-mode entry plans. The obsolete post-`#UD`
  transition has no public or runtime path.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T375 S18 | Accepted P1 `8a4487e1`: explicit virtual-time publication emits a distinct core trace event; persistent replay and Model-339 physical timing remain open. [Evidence](../etc/evidence/t375-s18-external-time-trace.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine capability ledger and receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: selected Model-339 device-service ownership is reconciled; unavailable duration transfers to phase refinement. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: accepted 5170 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility; board/device timing remains open. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: source-labelled 80286 successful-retirement timing closes; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: VM owns concrete machine selection and CPU/timing contract binding; no CPU-timing, bus, device or L3 receiver closes. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |

## Recent Governance

- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
