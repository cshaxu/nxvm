# Project Status

## Current Work

**M5 T267 S1 active: Standard PS/2 AUX Configuration And Status Protocol.**
T267 admits only the standard PS/2 configuration/status command plane through
the existing per-machine core KBC, AUX FIFO, IRQ12, and guest port route.

### Original Request

Implement `F3h` sample-rate configuration, `E8h` resolution configuration,
and `E9h` status response through `D4h -> 60h -> AUX FIFO -> IRQ12 -> guest`.
Preserve reset, identify, reporting, and three-byte stream-packet behavior.
Generate `build/output/nxvm_0_5_0267.exe` after S3 verification and push the
normal Git history.

### S1 Contract, Scope, And Risks

| Requirement | Owner and frozen behavior | Evidence / stop condition |
| --- | --- | --- |
| AUX defaults | `core/machine/kbc` owns per-machine stream mode, reporting disabled, scaling 1:1, resolution `2`, sample rate `100`, device ID `00h`, button state, and no pending AUX parameter. `F6h`, `FFh`, and cold reset restore all defaults and clear a pending parameter. | Core port probe observes defaults and reset. Stop if another module must own AUX configuration. |
| `F3h` / `E8h` | `F3h` replies `FAh`, then consumes exactly the next `D4h -> 60h` AUX byte as a sample rate; admitted rates are `10,20,40,60,80,100,200`. `E8h` likewise admits resolution `0..3`. Valid input replies `FAh` and updates state; invalid input replies `FEh` without changing state. Each outcome clears the pending parameter. | Core port probe observes sequence and atomic invalid state. Stop if a host or firmware path is needed. |
| `E9h` | Replies, without interleaving, `FAh, status, resolution, sample-rate`, all with AUX origin. Status defines bit 5 reporting, bit 4 scaling (`1` only for 2:1, therefore clear in this task), bits 0--2 current buttons; stream and remote bits remain clear. | Fixed KBC-owned delayed reply capacity is raised from 3 to at least 4; FIFO pressure delays but never drops the full response. |
| IRQ / serialization | AUX responses and stream packets are serialized by the existing KBC FIFO. A delayed command reply prevents host report admission; the response drains as a contiguous AUX sequence, every byte gets OBF/AUX and IRQ12, and `60h` read plus PIC EOI allows the successor edge. | Probe covers every byte, IRQ12, EOI/deassert, and reply-before-packet ordering. |
| Guest evidence | An owner-built DOS fixture installs IRQ12, configures sample rate/resolution through ports, validates the four `E9h` bytes from guest memory, then receives one host-originated packet. | Stop if it requires direct DOS API/BDA/RAM injection or a second AUX state. |

**Deferred:** wheel IDs and sample-rate handshake; four/five-byte packets;
scaling effect; `F0h`, `EBh`, `FEh` resend semantics; host capture and
wall-clock mouse timing.

**Applicable rules:** core owns device state and has no VM dependency;
platform does not mutate guest state; one KBC state owner and one IRQ12 route;
fixed/copy-owned queues only; no host-clock guest shortcut; retain NXVM
Console/debugger/boot UX. No external code or assets are imported.

**Similar-issue sweep:** inspect every AUX command and delayed-response site in
`src/core/machine/kbc.*`, the core AUX port corpus, and both guest AUX/mouse
fixtures. Classify response-capacity, pending-parameter, and reply ordering
sites; add focused regressions for every production path in scope.

**S1 commands and expected markers:** build/run
`core-machine-kbc-aux-port-smoke`, expecting `M5:T267:S1:AUX:PORT:OK`; static
ownership gates remain part of S2/S3. S1 must complete before implementation.

## Current Technical Baseline

- **T266 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0266`; static/ownership
  checks and 102/102 CTest cases passed. Artifact `nxvm_0_5_0266.exe` SHA-256:
  `4A9BADD8FFD8F26F4EC2DAA2D576C7D24D0AEA0072F6A5AD2D5ABCC15960928A`.
- **Core boundary:** T243--T246 retain checked physical memory, bounded `#UD`
  transitions, immutable ROM mapping, and atomic real-mode entry plans.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T264 | Closed PC/AT ownership debt without changing retained NXVM behavior. |
| T265 | Added bounded Level 2 instruction-cost attribution with zero default-PC/AT surcharges pending calibration evidence. |
| T266 | Closed bounded VADP CRTC cursor/page/status behavior and copied column/row coordinates. |

Detailed contracts, commands, artifact provenance, and prior closures are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S30:** compacted completed M5 planning records and formalized the
  T264--T278 forward queue.
- **M5 Td S31:** made `status.md` the sole current-baseline authority, retired
  completed T264--T266 records, and added a documentation-governance gate.
  This task changes no runtime behavior or active-task scope.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Project Status](status.md) and [M5 convergence queue](m5-pcat-hardware-convergence.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
