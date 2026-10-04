# T540 S93 Whole Board Receiver

Baseline: accepted S92, d76d2b15e. This is the receiving design for the active
S93 packet, not evidence of implementation or acceptance.

## Measured Dependency Knot

The remaining App board has one allocation containing six clock domains,
FDC/HDC/video adapters, AT keyboard and XT PPI wiring, parity/speaker state,
and DeskPro D4 memory/platform/refresh state. `machine_board.c` constructs,
resets and releases it; `machine_plan.c` applies its frozen topology;
`board_advance.c` and `board_deadline.c` consume the same private layout.
Moving either time file alone would create a Shared-to-App dependency.
Moving the layout unchanged would falsely make D4 a common PC mechanism.

S93 therefore consumes the whole dependency knot. Its working phases are not
separate subtasks, accepted partial receivers or additional public wrappers.

## Source And State Receivers

| Existing class | Receiving owner and boundary |
| --- | --- |
| FDC drive mechanics/media conversion, DOR/DIR/CCR routes and IRQ/DMA wiring | ibmpc-common, with copied config/media bindings and opaque adapter state; the 8272A remains chip-owned. |
| HDC port personality/media/IRQ/DMA conversion | ibmpc-common, with the existing explicit protocol descriptions and opaque adapter state; no second CHS or media cache. |
| Video PC port/memory mapping and snapshot adaptation | ibmpc-common; one video chip and its copied snapshot path, no renderer or product policy. |
| 8042/keyboard/AUX routing and A20/reset signaling | ibmpc-at, through public Core signals and PIC leases, not a private common-board layout. |
| XT PPI/DIP/keyboard/IRQ/NMI/speaker-line routing | ibmpc-xt, through public chip/Core contracts and bounded line callbacks, not a common-board-state include. |
| Common clocks, topology validation/application, construction rollback, reset phases, deadline reduction and finalization | One common board composition/lifetime owner. It integrates the actual public family contracts; no chip-layout imports or second Core implementation. |
| IBM planar parity and AT Port B semantics | AT family owner, distinct from D4 despite their shared port number. |
| DeskPro D4 memory decode, Port B/failsafe/IOCHK and refresh latches | Model40 Profile owner, not renamed generic AT. Its optional integration must use the existing bounded Core attachment lifetime and explicit board signal operations. No global lookup, model switch, generic device registry or raw board getter is eligible. |
| Product guest input/display conversion and firmware/media choice | NXVM driver/Profile, using opaque board handles and copied values. |

Only `*_interface.h` crosses component boundaries. Private state may remain
cohesive within its real owner; files are not split merely to shrink them.
Public family/controller handles own their allocations and never expose a chip
or mutable layout to the common board. The common composition owns their
lifetime and releases them before its Core attachment is finalized.

The source move preserves symbols where their meaning remains correct.
Resolved config values may move with the shared mechanism; product selection,
asset paths, ROM bytes and source provenance do not move into those values.

## Construction And Profile-Specific Integration

Core keeps one guest timeline, route table and copied attachment binding.
The board keeps one unpublished construction candidate and one rollback path.
Profile-specific D4 integration is a concrete composition requirement, not a
reason to add a pluggable bus or a second lifecycle owner. Its actual typed
operations must be derived from all existing reset, deadline, refresh, NMI,
speaker, memory-admission and destruction consumers before that cut is coded.
Any proposed seam which only conceals private state or recreates a general
device framework fails this design review.

The measured D4 receiver uses one copied `board_profile_interface.h` binding:
four existing reset phases, NMI refresh, refresh request/complete, an absolute
source-axis deadline and finalization. Frozen refresh-output ownership and
shutdown-reset flags replace model tests. Its one context owns D4 RAM and
platform state together; the common board never reads their layout. The
binding is published once during construction, cannot be replaced, and is
finalized before borrowed PITs. No registry, new dispatch loop, peer getter
or second Core attachment is introduced. Product-specific construction and
diagnostics must move out of the shared receiver before final relocation.

A family component never reads a peer's private header. The common composition
root owns the keyboard PIC leases and supplies copied IRQ line callbacks to
AT/XT adapters. Thus ibmpc-common can compose the public family contracts while
the family implementations depend only on Types, neutral Core and their chips;
they do not depend back on the common PIC bus. This removes the current
family-to-common lease dependency rather than adding a cyclic aggregate target.
The root binds family input/output and Core signal contracts explicitly.
No host time, Common control queue or native platform service enters this graph.

## Complete-Batch Proof

All original common/family hardware and failure assertions retain a receiver.
App tests that prove firmware/profile composition remain App-owned; tests of
the extracted component build independently in test/x86. Private-mechanism
tests stay with their actual owner, not with a peer that imports its layout.

Before acceptance reconcile every remaining App devices file and every board
layout reader, then verify source-list uniqueness, no Shared-to-App dependency,
independent tools-on/off builds, complete dual-width units, specialized gates,
all manifests, documentation, eight fresh product hashes/sole-Core links and
eight unchanged-INI checkpoints once each. A working intermediate relocation
is not an accepted component and cannot satisfy this completion predicate.

On 2026-10-03 the owner superseded the oversized S93 execution packet with
planned S94-S97: actual source/coverage review, independent verification,
eight-product verification, then complete delivery and T-level closure review.
See the [revised proposal](../../history/M5-T540-shared-ibmpc-integration-proposal.md#remaining-acceptance-s94-s97).
This design and its complete-batch predicate still apply across those tasks;
the split does not accept S93 or create partial component implementations.
