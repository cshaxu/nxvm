# T540 S84 Board Display Handle

## Scope And Executor Confirmation

The executor confirms CURRENT's S84 packet against accepted S83 P2
`ca0994e7340d6360ff0fc6a82c479702f4143063`. The complete three-operation
display class is received here; this is NXVM-only, not physical Shared
relocation or a timing qualification. Source authorization and the existing
embedded-product exception remain; no external master, INI or MyNES change
is permitted.

## Ownership And Similar-Issue Sweep

Configure, observe and capture receive the constructor's actual borrowed
board handle. VADP remains the unique video-state/cache owner. Core's existing
configuration-open check moves its declaration to the public neutral contract;
its implementation and return type do not change. Lifecycle observation uses
the board's opaque Core handle. Capture preserves its original const receiver
and owner-local mutable cache cast, not a second snapshot/cache path.

The internal topology application now receives the already constructed board.
Its display call and two DMA-request fields use that same allocation. Other
configuration calls still retain their existing Core receiver for their later
whole-class cut. No getter, forwarding wrapper, new function, allocation,
registry, destructor or runtime rebinding is added.

The sweep searches every tracked NXVM source/test C/H file for
`core_machine_(capture_display_snapshot|observe_display_snapshot|configure_display)`.
The baseline has 36 calls, three declarations and three definitions in 23
files; every original call is migrated or is an existing null-receiver case.
Three new regression calls bring the current total to 45 symbol occurrences,
including those six declarations/definitions. Seventeen production/integration/
unit caller files equal baseline plus exact receiver substitution. The three
remaining unit fixtures retain all original assertions while acquiring the
actual board output or extending null/configuration/reentry checks.

Display no longer includes private machine.h or recovers `->board`.
The existing display-authority gate checks all three declarations/definitions,
public Core guard ownership, constructor topology and all NXVM C callers.
Positive copied-source proof and injected negatives verify those checks.
Copied probe state is bounded below build and removed after proof.

## Verification And Delivery

Implementation verification is complete; coordinator acceptance is below.
Complete units pass 470/470 in both widths: x64 289.17 seconds, x86 94.19
seconds. Their serial CPU-boundary negatives pass in 63.60/66.67 seconds.
Complete specialized gates pass in both widths, including the 402-row strict
matrix (377 strict, 25 deferred). Intentional negative self-test diagnostics
are caught as expected. The controller gate initially matched `core_machine_config`
inside the new `core_machine_configuration_is_open` name. It now matches
complete identifiers, retaining include-path checks. Five injected board-type
negatives remain rejected, while the neutral eligibility operation passes.
This is a verifier correction, not a production compatibility branch.

An initial restricted Ninja process remained live
without compiler children or output; an independently successful dry-run
showed the outstanding work. Its exact process identity was checked and
the owned process was stopped before the normal-permission build.
The early execution of pre-existing test binaries is not counted as S84
verification. Rebuilt regressions passed. Sixteen injected display negatives
and five exact-token contract negatives pass; copied probe inputs are removed.

Source/tests occupy 26 changed paths, +79/-60, net +19. Production alone is
seven paths, net +5; tests add 14 net lines. The two existing gates add 39 net
lines. No new production function, state or execution route is introduced.

Every deployed product passes PE width, 0.5.0540 banner, source freshness,
optimized Release and compiler-debug-section absence checks. The two default
boots reach the DOS prompt; XT, AT and Model 40 pairs reach the running
installer. All eight rebuilt independent neutral executions also pass.
Each unchanged INI boot runs exactly once, with a 180-second containment
limit, not a claim of permanent absence of intermittent faults.
Documentation governance, all 466 changed-document links and diff checks
pass. Shared's six corpora, MyNES, INIs and external masters are unchanged.
All owned verification commands have exited; ignored caches/scripts/logs
remain needed by the next receiver, but copied negative probe inputs are removed.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1337080 | `45DB48327CC2BC2CB96C1526AB6EEA36FFE8503714DB782885653434DDD0C027` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1507400 | `05826DC5401C8ABBF3A9CF2EA4FEF44CF4C11A7838EFD881F317548A2A109250` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1353397 | `F692D5614AEAEBF66882283BCB5B264493E413E7D2ED273E97A5682086E68923` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1523715 | `5898E20052219953AAFCA1ECB7D5F5F6BAC919FD51FF7A45749A4B5636E8F7C1` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1353365 | `B5F9EBA994FB43A724B5D3D9B61FC317BF06E113CBD5A7A051AF94E2F4D84468` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1523682 | `907BA6928682A05B84BF6AC2D9049E04CC8645233147A2DB15F67D0D12D60864` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1353431 | `78B4071AA6D5A1CC70372A6B28E50BEABF3AB442BBE5D6ADFBABA125562BDE3B` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1523750 | `6B72C9C8D316A4B896A4C2E1F6D3E1069D052E1AE0E809400FBBE6462BC4B124` |

## Remaining Receivers

Other public board configuration/observation operations, chip-wiring contexts,
direct-board fixture classification and Core's private board association
remain open. Physical neutral Core and flat IBM-PC relocation is still
required. S84 does not close T540.

## Coordinator Acceptance

P1 `d23281d1badeaaf5887f35496ff64bb0218f0845` was immediately pushed to
origin/master. Actual committed-diff review covers all 41 paths: seven
production files, nineteen tests, two gates, five documents and eight EXEs.
Seventeen whole caller files differ only by receiver substitution; Core's
guard/lifecycle implementation is unchanged. Display's cache, CECG validation,
status ordering and paused/stopped observation retain the original behavior.
The three extended fixtures retain their original assertions and constructor
ownership. The gate correction rejects exact board types without rejecting
the neutral guard's longer identifier. All committed blobs match their
reviewed files, and the eight evidence hashes match committed artifact bytes.

The full packet, original owner request, numeric allocation, complete receiver
sweep, rules, test results, sole finalizer and remaining ledger agree. No
corrective implementation P is needed. Governance P2 accepts S84 and removes
only its active packet; T540's other public board operations, wiring contexts,
direct fixtures and physical relocation remain open.
P2's three governance-only deltas pass documentation checks, all 446 current
changed-document links and diff checks; no executable input changes again.
