# T369 S3: PC/AT Logical HOLD Lifecycle

## Accepted implementation boundary

P1 commit `12ac65ac` adds one core-owned, source-labelled **logical** DMA
HOLD lifecycle for the 80286 execution-round arbitration boundary.  It is
enabled only for the 80286 profile and only when the existing DMA clock
publishes a tick and the dual-8237 owner has a pending request.  The lifecycle
is:

```text
request -> acknowledge -> same-owner DMA transaction -> release
```

`transaction.[ch]` owns validation and state.  A pending hold rejects a CPU
transaction; an acknowledged hold admits only the same DMA owner.  The
arbitration callback in `machine.c` remains the one DMA publisher and releases
the hold after its existing DMA advance.  Transaction reset cancels an active
transaction first, releases a pending hold, then clears state while preserving
the copied trace binding.  No new scheduler, CPU microstep/resume path,
stopped/paused API behavior, device-local route, or elapsed-tick publisher was
introduced.

The trace adapter copies request, acknowledge and release events through the
existing transaction trace boundary.  The focused competition smoke proves a
CPU transaction is rejected while DMA owns the logical hold, and proves both
the normal and reset order:

```text
CPU commit < CPU retire < HOLD request < HOLD acknowledge
    < DMA begin < DMA commit < HOLD release

reset-path HOLD request < HOLD acknowledge < HOLD release
```

## Verification and artifact

On 2026-08-14, the following focused build and execution completed successfully:

```text
core-machine-transaction-s2-smoke.exe
M5:T354:S2:TRANSACTION:OK

core-machine-competition-s3-smoke.exe
M5:T354:S3:COMPETITION:OK
M5:T369:S3:PCAT-HOLD:OK

core-machine-transaction-lifecycle-s4-smoke.exe
M5:T354:S4:TRANSACTION-LIFECYCLE:OK
```

The current-artifact verifier reports
`M5:T197:S1:CURRENT-ARTIFACT-TARGET:vm-0-5-0369:OK`.  The locally retained
developer artifact is `build/output/nxvm_0_5_0369.exe`, 2,806,784 bytes,
SHA-256
`460B470378405F9BB958E75B95F379FC8726BCAAB40AC83267CAD8F10DC2C121`, built
from P1 source commit `12ac65ac`.  Its compiled runtime identity is
`Neko's x86 Virtual Machine [0.5.0369]`; it is a local developer artifact, not
a product release and contains no guest media or firmware.

The similar-route sweep used:

```text
rg -n "core_machine_transaction_(begin|commit|cancel|hold_)|core_machine_dma_(has_pending_request|advance_transaction|request_)|CORE_MACHINE_TRACE_TRANSACTION|core_machine_(cold_reset|reset)" src tests CMakeLists.txt
```

The transaction begin/commit/cancel production calls remain in the sole core
transaction owner; DMA request/advance remains in `dma.c` and the one
arbitration callback; reset remains through `core_machine_cold_reset`; and the
only new trace consumers are the focused copied-trace assertions.  External
memory/port operations are excluded because they occur outside executing guest
rounds and retain their established immediate lifecycle contract.

## T366/T369 closure and transfers

This S records, but does not reopen, the accepted T366 prerequisites:

| Accepted fact | Durable boundary |
| --- | --- |
| 5170 baseline | IBM PC/AT 5170 Model 339 / Type 3: 8 MHz 80286, Rev.3 1985-11-15 firmware slots, 512 KB planar RAM, 101-key keyboard, CGA, no fixed disk, and the separately identified compatible TEAC 1.44 MB field upgrade. |
| NMI and devices | Planar-memory parity is the selected NMI source with its mask/latch/reset contract; the selected topology and device route audit are complete prerequisites, not device-service timing results. |
| CPU rows | T366/T368 accepted the bounded 80286 successful-retirement source rows. Their instruction costs remain the sole CPU time publication; this S neither alters nor completes physical CPU timing. |

The following remain explicitly open and are transferred, not implied by this
logical handoff:

| Open boundary | Receiver |
| --- | --- |
| Model-339 memory, ROM, CGA, ISA and port READY/wait values; maximum HOLD latency; physical HOLD/HLDA, DREQ/DACK/AEN and INTA waveforms | Remaining T369 bus/phase work after a new admitted S. |
| FDC/TEAC controller, DRQ-to-grant and mechanical service duration; other selected device-service timing | T370 device-service timing corpus. |
| Remaining source-undefined/range CPU work and physical CPU/bus accounting | Ordered CPU and bus closure candidates; no row is converted to a cycle claim here. |
| MFM/ST-506 fixed-disk controller and drive | Existing TODO admission path; current ATA/HDC is not relabelled as IBM storage. |
| 5170 Model-339 L3 closure audit | The ordered 5170 phase and final-audit candidates, only after their CPU, bus and device prerequisites. |

Therefore T369 S3 closes only the admitted logical HOLD lifecycle.  **IBM
5170 Model-339 L3 precision is not complete or claimed.**
