# Project Status

## Current Work

NXVM-hosted [T562 S3](../../nxvm/states/CURRENT.md) admits MyNES as a receiver
of Shared KVM cursor-shape and Emulator fixed-command metadata consolidation.
The S3 correction additionally admits the real Machine pacing/PCM receiver
regression and rebuilt artifacts for the Emulator heartbeat fix. No MyNES
production, configuration or snapshot change is admitted. Earlier delivery
source was `c81762c5f`; corrective Shared delivery is `86175d302`. Both
widths pass all 631 repository units and 12 MyNES integrations, including
automated native Window/Console tests. Both 0044 EXEs are rebuilt and recorded
in the [correction inventory](../../nxvm/etc/evidence/t562-s3-correction-artifacts.json).
The first x86 native-console integration failed its Escape/pause assertion;
unchanged isolated and complete-suite reruns passed. Its unproven cause remains
in the review ledger; it is not claimed repaired.
The [S3 evidence](../../nxvm/etc/evidence/t562-s3-correction.md) distinguishes
background/automated desktop proof from unperformed manual RDP acceptance.
Earlier hashes below remain historical; RDP input is not requalified.

No active MyNES S. On 2026-10-13 the owner requested T556 S10 closure.
S10 is closed as a bounded diagnostic delivery with the unresolved repair
transferred to [TODO](TODO.md#rdp-short-input-repair), not as a production fix.
The [investigation](../etc/evidence/t556-s10-rdp-controller-investigation.md)
retains dual-width reproduction, diagnostic delivery 3dc750be2 and verification
limits. Actual mobile RDP input capture and repair acceptance remain unproven.
A future repair needs a new admitted task; none is started by this closure.

The accepted T557 S2 receiver is 08bdaf06d: explicit disabled cursor mode and
rebuilt 0044 EXEs. Both widths passed 592 repository units and 12 MyNES
integrations. This cursor change does not repair RDP short-input semantics.
This governance-only closure changes no executable inputs; those releases
remain current and need no rebuild.

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
