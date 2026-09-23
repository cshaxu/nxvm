# Roadmap

The owner approved M0-M5 on 2026-09-19 and separately admitted M6 on
2026-09-22. Dependency order is M0 -> M1 -> M2 -> M3 -> M4 -> M5 -> M6.
Boundary approval is not milestone admission
or implementation authorization. T decomposition occurs only when admitting
that milestone, under [Execution](../../rules/EXECUTION.md). Current owns actual
admission/closure state. This document allocates no T/S identifiers.

## M0: Governance, Architecture And Planning

Scope: coherent goals, architecture, coding/source conventions, interaction,
governance, milestone boundaries and admission/verification discipline.
Existing documentation work continues as successive S units in the same Td.

Exit: authorities and approved milestone boundaries agree; execution admission,
task decomposition and evidence rules are reviewable. M0 does not require M1
or later T queues to exist. Source/test import, scaffolding and runnable product
work are excluded. Its closure is distinct from admitting M1.

## M1: Shared Lib/Common Foundation

Requires M0 closure and explicit M1 admission. Scope: provenance-reviewed,
unchanged adoption of the four SoftPC Lib/Common source and test roots, their
independent builds and complete Windows x64/x86 regression evidence.

Exit: fixed-source identities and rights are recorded, all four roots match
the selected upstream snapshot, and aggregate/independent suites pass. Upstream
x86 separation is consumed as delivered; MyNes does not repeat that extraction
or introduce optional x86 components merely to prove reuse. Actual downstream
reintegration, exhaustive host qualification and NES product integration are
not claims of this milestone. This boundary revision follows the owner's
2026-09-20 direction to close shared import and begin App/Core design.

## M2: NES Execution And Management Foundation

Requires M1 acceptance and M2 admission. Scope: App/Core integration, transactional
ROM loading, CPU/bus and initial cartridge mapping, cooked Console management
and basic NES debugging. The admitted design task fixes exact hardware/ROM/opcode
coverage before implementation candidates may execute.

Exit: controlled ROM/tests load and execute without graphics; run/pause/reset,
step and copied observation use the production executor; CPU behavior and rejected
inputs/failure cleanup have bounded proof. No graphics/audio/gameplay claim follows
from this milestone.

## M3: Dual KVM Graphics And Input

Requires M2 acceptance and M3 admission. Scope: initial PPU/controller/frame
publication, Window pixels and Core-produced text frames for Console ASCII,
keyboard configuration, surface switching and return to cooked management.
Core output adaptation owns graphics-to-text conversion; shared Lib/Common
retain text-only Console presentation and existing frame-kind mode switching.
Core publishes the representation that this existing Common mechanism consumes;
no NES format-negotiation layer or local Lib renderer is introduced.

Exit: the same controlled scenarios display and accept input through both KVM
backends, with Core-generated text reaching Console through the production frame
path and bounded conversion proof; switching never reloads the guest; focus loss,
pause and rebinding do not retain stale keys. Both presenters must pass. Coding may be sequential,
but Console cannot be left as an optional follow-up. Sound and broad game
compatibility are not required for this graphics/input closure.

The owner also requires correction of M2 architecture and boundary deviations
within M3. Its exit includes restored App/Core isolation, safe resource lifetime,
transactional publication and single hardware-time ownership, with all applicable
items in the [remediation ledger](../etc/m3-remediation.md) closed by direct
production evidence. Graphics success alone cannot waive these corrections.

## M4: First Supported Games With Complete Experience

Requires M3 acceptance and M4 admission. Scope: bounded initial game coverage,
APU/audio output, host pacing and hardware timing needed by that coverage;
resolve defects exposed by the declared gameplay flows.

Exit: the named support set completes specified play scenarios on both KVM
backends with accepted sound, speed, input and management behavior. Game/fixture
provenance and permitted observations are explicit. CPU tests or a visible title
screen cannot substitute for this evidence. Coverage is fixed at admission,
not an unbounded promise to run every game.

## M5: Compatibility Expansion And Release Baseline

Requires M4 acceptance and M5 admission. Scope: a finite extension of mapper,
ROM-format and hardware coverage; persistence appropriate to supported cartridges,
configuration saving and reproducible release delivery. Specify the expansion
set and persistence semantics at admission; save states are not implied.

Exit: the declared regression/support matrix, known limits, reproducible builds
and release evidence agree on one version baseline. This is not all-NES
compatibility. Save states, replay, rewind and networking are outside this
approved mainline unless separately proposed and admitted.

## M6: Snapshot State

Requires M5 closure and M6 admission. Scope: a versioned, bounded, transactional
snapshot of one supported NES cartridge and all mutable machine state, with
SoftPC-style `save <file>` and `load <file>` control-console behavior. A save
may capture a running or paused machine and leaves it paused. A load is admitted
only while stopped, validates all sections and the referenced cartridge before
any live-machine mutation, then leaves the restored machine paused. A paused or
stopped machine without an existing Window must not cause Window creation.

Exit: a single, documented on-disk schema names its magic/version, immutable
cartridge identity/reference, bounded sections, byte order, validation and
compatibility rule. Core owns snapshot capture/decode/restore of guest state;
App owns file paths, command wording, atomic replacement and lifecycle routing;
Lib/Common remain neutral byte-stream/lifecycle providers. Malformed, truncated,
unsupported-version, incompatible-cartridge and write-failure cases preserve the
prior machine/state according to their documented boundary. Both x64 and x86
pass deterministic round-trip, failure-atomicity and cross-process proof.
Replay, rewind, save-state slots, compression, ROM redistribution and backward
compatibility with unversioned experimental files are excluded.

Every later Common change preserves the unchanged-transfer contract and carries
its own tests. A needed Lib capability is developed upstream and adopted through
a reviewed shared revision, never a product-local patch. Each milestone must name
measurable coverage and verification
when admitted; later milestones remain goals/dependencies/exits until then.
