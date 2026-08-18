# T403 S1: Keyboard Serial Endpoint And 8042 Flow Control

`M5:T403:S1:KBC-FLOW:OK`

## Decision

The shared Core KBC now separates accepted native keyboard serial bytes from
the CPU-visible 8042 output FIFO. This corrects a functional input-loss defect
used by the DeskPro composition without asserting a physical keyboard, 8042 or
Compaq FIFO depth or timing.

## Producer and consumer matrix

| Producer or state | Owner and disposition |
| --- | --- |
| Mapped host make/break sequence | VM maps host transitions only; Core atomically accepts the complete native sequence into its private serial endpoint or returns capacity exhaustion without partial admission. |
| Native Set-1/Set-2 break and typematic state | Core endpoint admission owns it. A matching accepted break cancels typematic before output publication. |
| CPU-visible `60h` output, `64h` status and IRQ1 | Existing Core KBC FIFO remains the only CPU-visible output owner. Dequeue promotes serial bytes in arrival order and retains IRQ refresh. |
| Keyboard/controller delayed responses and AUX output | Existing Core KBC output/response/AUX owners remain unchanged. The endpoint drains before delayed responses are promoted, preserving existing output ownership. |
| Reset, scan enable/disable, translation and command state | Existing KBC owner remains unchanged; reset clears endpoint state with the rest of private KBC data. |

IBM PC/AT and Intel 8042 material remain the primary observable controller
boundary. The separate bounded serial endpoint is a project-owned generic
skeleton needed to preserve an accepted sequence under output congestion; it
is not a claim about physical buffer depth. No third-party source is copied.

## Repair and proof

The old single-FIFO path could reject an entire `F0 1E` Set-2 Enter break when
CPU-visible output was full. That left Enter typematic active. The new endpoint
accepts the complete break atomically, cancels typematic on admission, then
promotes the pending native bytes as `60h` space becomes available.

`core-machine-kbc-controller-smoke` now fills CPU output with non-typematic
`E0h` bytes while Enter typematic is active, submits `F0 1E`, verifies immediate
typematic cancellation, drains the pre-existing output, then observes the
break in order before the queued command response. Related KBC/AUX, mapper and
DOS-keyboard smokes pass.

## Transfer

Physical DeskPro keyboard serial rates, actual 8042 firmware, buffer depth,
wire-level flow control, board IRQ latency and Model-L3 timing remain
unmodelled. They transfer to the later physical-observable device timing
receiver. This repair adds no profile-specific Core path and no Core/VM public
interface.
