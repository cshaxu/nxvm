# T499 S2: D4 8042 Speed-Path Source Correction

## Finding

The previously selected D4 path interpreted generic 8042 command `D1h` output
bit 3 as a gate for an auxiliary-PIT ``slowdown`` state.  This is not a
source-qualified Model-40 relation and has been removed.  Core still owns the
generic 8042 output port and its A20/reset semantics; this correction creates
no VM-side speed path.

## Original-source classification

| Row | Original-source result | Current disposition |
| --- | --- | --- |
| `D1h` output-port write | The DeskPro 386 Technical Reference, section 2, states that system-speed bits are not set through this command and directs speed handling to `A1h`--`A6h`. | Manual-L3 negative result: no D1-to-speed coupling. |
| `A3h` / `A4h` | The same source assigns enabling/toggling system-speed control to these command paths. | Manual-L3 functional rows; not implemented by the removed D1 path. |
| Physical speed duration / Core tick conversion | The inspected source names speed modes but supplies no complete mapping from Core's source ticks through CPU/bus execution and board logic. | L1 timing gap; no estimated deadline or gate is permitted. |
| Auxiliary PIT | Existing D4 evidence supports its failsafe connection, not a D1-driven speed gate. | Retained at its existing owner and provenance. |

The read-only 86Box and PCjs DeskPro models inspected for this task do not
provide a corresponding D1-to-auxiliary-PIT speed model.  They therefore do
not supply an External-L2 substitute.

## Code correction and replay result

`core_machine_d4_platform_config` no longer accepts a slowdown PIT counter;
the D4 refresh callback no longer changes an auxiliary-PIT gate, and the Core
run loop no longer advances a D4-specific one-tick hold.  The focused D4
platform, refresh-hold, competition and prefetch regressions pass after the
removal.

The owner-authorized read-only DeskPro ROM and 1.2-MB DOS media replay now
creates the session and executes BIOS code after the correction.  It has not
yet reached the FDD command path or a DOS terminal, so this is progress
evidence only and not S7 matrix success.  The remaining failure is being
investigated at the Core memory/firmware boundary; no profile-side workaround
has been added.
