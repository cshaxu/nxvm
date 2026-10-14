# Project Status

T557 S2 is an owner-approved Shared cursor-contract receiver coordinated in
NXVM Current. MyNES changes only the text adapter's hidden cursor value to
KVM_CURSOR_DISABLED and rebuilds its existing 0044 pair against that contract.
This does not repair or close the independent S10 RDP input investigation below.
Both rebuilt EXEs pass the final 592-unit repository run and all 12 MyNES
integrations per width; native desktop tests are included, phone-RDP acceptance
is not. The owner accepted and closed T557 S2 on 2026-10-13; this does not
close or alter the independent MyNES S10 packet below.

## Current Work

MyNES M5 T556 S10 is admitted for the owner's RDP game-input report.
The owner explicitly identified MyNES, not the four PC Apps. The hosted
[proposal](../../nxvm/proposals/m5-app-component-layout.md) records this bounded
corrective extension. S9 remains delivered; this admission does not claim its
manual acceptance or close T556. Keep the accepted Core/Machine/Product layout.

## Current Task - M5 T556 S10 RDP Game Input

| Field | Required record |
| --- | --- |
| Identifier Mode | Owner-added numeric S10 after delivered S9; hosted by NXVM T556, MyNES implementation target. |
| Admission And Approval | Owner requested a new S to investigate and repair RDP game input, then clarified MyNES. Shared implementation changes still require a concrete review before editing. |
| Objective | Locate and repair the proven cause of lost WASD, J/K and potentially Shift/Enter game input under RDP through the existing input path. |
| Non-goals | No PC App changes, RDP-specific alternate input path, native API in MyNES, new input manager, ROM/INI/snapshot mutation or speculative hardware rewrite. |
| Reference Baseline | Clean master/origin baseline 7bfefb2fc; S9's released MyNES 0044 pair. |
| Candidate Proposal | [T556 host proposal](../../nxvm/proposals/m5-app-component-layout.md), S10 amendment. |
| Files And ABI Surface | Read-only audit of Lib KVM normalization, Emulator input dispatch and MyNES Product/Machine/Core controller. Owner-approved investigation delivery moves the opt-in owner-ROM probe to test/app-mynes/diagnostic and its diagnostic build aggregate, with strict warnings; no automatic CTest, production or public API change. Production repair remains separate pending work. |
| Applicable Rules | MyNES guide, architecture, coding, UI and source policy; shared Execution, Architecture, Coding and Document rules; architecture-governance and coding-governance skills. |
| Verification | Reproduce the failing event sequence before repair; distinguish physical held transitions from character pulses; cover source retirement, pause/resume and short presses. Run affected x64/x86 builds, MyNES units/integrations and relevant boundary gates in existing Ninja/ccache trees. Report actual RDP validation separately from synthetic input tests. |
| Expected Markers | All eight NES buttons reach the controller from the appropriate input route; make/break releases remain balanced; registered hotkeys remain consumed; no duplicate delivery or stuck input. |
| Asset Needs | Existing owner media and snapshot read-only; rebuild MyNES dual EXEs if production changes, retaining the independent 0044 version. Shared changes would require a separately reviewed affected-App release scope. |
| Reporting Requirements | Report diagnosis and reproduction, proposed files and estimated diff before repair, actual source/test diff, exact test results, artifacts and unavailable desktop/RDP verification. |
| Stop Conditions | Required Shared modification, unproven native event assumptions, incompatible snapshots or expanded product scope: explain evidence and request the missing decision instead of adding a workaround. |
| Exit Criteria | Proven defect repaired on the unique owner path, relevant dual-width qualification and affected artifacts complete, reviewed target-scoped commit pushed; owner tests RDP before S closure. T stays open. |
| Original Owner Request | Investigate and fix ASDW, J, K and possibly Shift/Enter failing to control games over RDP; product explicitly MyNES. |
| Similar-Issue Sweep | Check all eight button bindings, key/text normalization, source identities, frozen state and queued make/break ordering; do not infer the reported cause from isolated mapper tests. |

### Initial Investigation

The existing driver maps WASD, J/K, Shift and Enter; physical makes also retain
a transient press until the next controller latch. Existing adapter tests cover
paired short transitions and text pulses, but do not prove the owner's actual
RDP event stream traverses the native window and Session unchanged.

Against the unchanged S9 binaries, eight focused tests passed per width:
Lib `win32_keyboard`, `kvm_input_admission`, `kvm_keyboard_lifetime`;
Emulator `input_queue_mouse_fifo`, `input_queue_ownership`; MyNES
`core.controller-smoke`, `machine.media_failure_contract_smoke` and
`integration.controller-rom-smoke`. No production change or actual RDP
reproduction is claimed. The owner confirmed Window presentation and a mobile
RDP software keyboard. MyNES composition registers only unmodified Escape as
`pause-toggle`; its help also identifies Esc, not Ctrl+Alt+P. The earlier CAP
question incorrectly applied the PC product's binding to MyNES. Investigate
the Window character/packet route and controller sampling; do not assume the
phone supplies physical held-key transitions or that missing mappings explain
the report. The owner further confirmed that Escape works on that same mobile
software keyboard while game controls do not. This proves hotkey delivery, not
game input delivery: Session handles hotkeys before dispatching ordinary input
to Machine. The existing native-window integration checks a held K transition;
it does not establish mobile character-pulse behavior. Core consumes transient
buttons at the first controller latch.

The [bounded owner-ROM investigation](../etc/evidence/t556-s10-rdp-controller-investigation.md)
now reproduces the short-input defect on x64 and x86: the configured TMNT3
cartridge reads twice per frame, gets `08,08` for held Enter but `08,00` for a
character pulse, and clears its game input in the latter case. Actual mobile
native events remain uncaptured; this result does not exclude an earlier input
failure. Production, public six, INI, snapshots and EXEs remain unchanged.

The owner approved a bounded investigation-only delivery: keep the probe in
Diagnostic, verify both widths, and publish its evidence without claiming a
production fix. S10 remains open for a separately reviewed repair; NXVM may
proceed with its independently admitted PC naming task.

## Retained S9 Delivery

| Field | Required record |
| --- | --- |
| Objective | Align moved Machine driver/debug symbols with their actual owner and reconcile current layout documentation. |
| Scope | MyNES Machine declarations/callers/tests, product documentation and rebuilt 0044 pair. |
| Boundary | No Shared code, deterministic Core algorithm, persisted snapshot format, ROM, INI or media changes. |
| Verification | Both existing unified Ninja/ccache builds; all 45 MyNES units and 12 integrations per width; documentation and dependency gates; current EXE pair. |
| Exit | Actual-diff review and target-scoped pushed commit; NXVM S9/T556 remains governed by its own complete qualification requirements. |
| State | Implementation and dual-width 57/57 qualification complete; target-scoped delivery recorded by the S9 evidence commit. |

## Retained Evidence

T556 S2 was delivered as `693030c2e`: the Machine adapter owns Emulator
integration, storage/frame/audio/debug adaptation; Core owns deterministic
hardware. Both width-specific suites passed 57/57 at that delivery.
The `0044` identity belongs to MyNES's accepted independent version baseline;
S9 renaming does not change snapshot bytes or invent a new MyNES T number.

The [S9 receiving report](../etc/evidence/t556-s9-machine-owner-receiving.md)
records actual source/test diff, verification boundaries and current artifact hashes.

Earlier qualified artifact hashes and outcomes remain in the
[retained status archive](../history/M5-T556-retained-status.md).
They are historical evidence, not current hashes or concurrently active tasks.
