# T392 S1: C1 Existing-State Candidate Inventory

`M5:T392:S1:C1-CANDIDATE-INVENTORY:OK`

## Frozen universe

The candidate universe is every existing copied fact reachable by the retained
Model-40 test observer after M40-C0A: retirement sequence and copied execution
point; successful-retirement mode/privilege/prefix/form context; control and
repeat classifier context; test-local prior-observation state; and the existing
post-C0 immediate-I/O recognizer. Device internals, mutable session state,
firmware bytes, PC retention, raw traces and new callbacks are excluded.

| ID | Candidate semantic fact | Existing copied fact and reset/failure boundary | Result / receiver |
| --- | --- | --- | --- |
| C1C-001 | Second protected-mode entry after C0 | `protected_mode` edge plus test-local prior mode; cold reset clears it; S6 did not observe it within containment. | Not an admissible endpoint: a mode edge alone is not a boot transfer. Transfer to a future endpoint design only if paired with an existing finite consumer fact. |
| C1C-002 | Boot-sector control transfer | Existing copied `linear_pc` permits a test-local boolean only; reset clears it; S6/S7 did not observe it. | Not reached; containment is not a failure or endpoint proof. Transfer to future finite-endpoint design. |
| C1C-003 | First post-C0 immediate port-61h read | Existing opcode/port recognizer and D4 owner; reset/fault/form-capacity terminals are distinct. | Already consumed as M40-C0A; not a later C1 endpoint. |
| C1C-004 | Later ordinary I/O | Retirement form can identify immediate I/O, but DX-port operands are not published and a port number is unavailable. | Insufficient for a device semantic milestone. Earliest receiver is a separately admitted device-observability mechanism, not Core callback expansion. |
| C1C-005 | First HLT, loop, repeat or classifier transition | Existing successful-retirement form/control/repeat context; reset clears test aggregate; all are CPU-local and may recur. | Non-semantic diagnostic only; cannot identify firmware phase or boot readiness. |
| C1C-006 | Disk/FDC/HDC completion, IRQ delivery or BIOS result | No copied retirement fact contains device completion, request ownership, interrupt cause, BIOS result, or media semantics. | Blocked from this task: earliest receiver is a separately selected device/session observability design with its own source and ABI review. |
| C1C-007 | Session stop/fault/result | `core_machine_run_result` is a caller result, but only stop/fault state and linear PC are available; it has no successful boot criterion. | Not a startup endpoint. Existing fault/status outcomes remain failure boundaries only. |

## Completeness decision

All 7/7 existing-state candidates have an explicit result. None can form a new
finite C1 checkpoint: C1C-003 is already the C0A predecessor, C1C-001/002 were
not reached and cannot be promoted by their containment limit, C1C-004/006
need broader device/session facts, and C1C-005/007 have no consumer semantic.
No reproducible earliest-owner functional defect is identified by this
inventory. This task therefore neither requests nor justifies a Core/VM
interface expansion.

The next receiver is a T392 closure decision: retain 80386 physical-retirement
qualification as blocked, and transfer any future C1 endpoint work to a
separately admitted selected device/session observability design. The existing
S5 synthetic capture remains the reset/failure proof for the only copied-state
C1 boolean capability.