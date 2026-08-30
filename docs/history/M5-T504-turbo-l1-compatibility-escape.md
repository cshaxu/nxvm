# M5 T504 Turbo L1 Compatibility Escape

T504 is the bounded compatibility receiver for the remaining Core-owned L1
waits without a source-qualified deadline. It preserves one guest-time owner:
Core selects and orders known events, while VM selects Standard or Turbo and
observes copied status only.

Its first subtask inventories the entire reachable blocked/no-deadline
universe before any implementation. Later subtasks may add one shared,
bounded Turbo-only progression only when an owner's normal lifecycle remains
preserved. Standard does not use that progression and neither mode receives
guest time from the host.

No task result may promote causal-only timing to L2/L3, invent a controller
deadline, add a parallel scheduler, or retain a device-specific workaround.

## S1 accepted inventory and baseline

S1 P1 `3a39c7a8` inventories the finite blocked/no-deadline universe in the
[indexed evidence](../etc/evidence/t504-s1-l1-no-deadline-inventory.md).
Coordinator P2 review accepts the four L1 candidates (DMA, HDC, D4 refresh,
and unpublished slave-PIC cascade), separates already-due FDC work and external
HLT, and confirms the owner-local baseline corrections. Focused 4/4, complete
unit 312/312, and documentation governance pass. S2 owns only the copied Core
disposition boundary.

## S2 accepted copied boundary

S2 P1 `548fe25b` adds the copied `IDLE`, `DEADLINE`, `IMMEDIATE`, and
`L1_COMPATIBILITY` disposition documented in the
[indexed evidence](../etc/evidence/t504-s2-copied-time-progress-disposition.md).
Coordinator P2 review confirms that this is an observation-only public value:
it carries no device identity, pointer, or tick request, and it changes neither
the scheduler nor the VM runner. Focused 4/4, complete unit 312/312, and
documentation governance pass. S3 owns the sole Core progression action.
