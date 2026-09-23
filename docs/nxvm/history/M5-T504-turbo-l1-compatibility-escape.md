# M5 T504 Turbo L1 Compatibility Escape

The retained [admission proposal](M5-T504-turbo-l1-compatibility-escape-proposal.md)
records the original bounded scope and constraints.

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

## S3 and S4 accepted Core progression and lifecycle proof

S3 P1 `2c19117d` adds one construction-frozen profile policy and one
Core-owned, bounded L1 progression action. Before every owner transition, Core
re-observes the complete state, so immediate or sourced-deadline work wins and
VM provides neither a controller choice nor guest ticks. S4 P1 `28235d06`
then proves the Standard/Turbo and lifecycle guards: Standard never requests
the action; Turbo does so only for the copied L1 disposition; stop, pause,
reset, debugger, input, interrupt and sourced-deadline paths retain their
normal boundary. The Core-only source-less fixture proves the L1 transition;
supported profiles correctly observe their PIT source deadline instead.

## S5 accepted integration and artifact closure

S5 P1 `939dfce1` records contained same-input external-ROM/DOS replays in
[its indexed evidence](../etc/evidence/t504-s5-integration.md). Both Standard
and Turbo reach the same DOS terminal. Turbo is faster in the paced default
PC/AT replay; the unqualified Model-40 replay has identical semantics but no
physical/macro pace to remove. These observations do not claim physical timing
or convert host elapsed time into guest time.

P2 `79e6b2c0` corrects the separately discovered Release-artifact delivery
defect: obsolete, rejected YAML was beside the executable but no current
templates were emitted. The one strict parser remains unchanged; the target
now copies two versioned, media-free current-grammar templates beside every
Release executable. The formal `0504` artifact lists both templates and opens
the selected default-PC/AT session. Complete unit, integration and
documentation-governance gates pass.

The finite T504 universe is closed: its shared Turbo escape remains only the
safe L1 fallback, and its four owner-specific timing receivers are explicitly
transferred to the queued PIC cascade, DMA service/arbitration, HDC
personality-service and Model-40 D4 refresh-hold tasks. They must establish
their own source-led List 1/List 2 evidence before they may replace an L1
disposition. No T504 controller timing relation is promoted by the escape.
