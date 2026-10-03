# T540 S85 Board Controller Handles

## Scope And Executor Confirmation

The executor confirms CURRENT's S85 packet against accepted S84 P2
`dceff7243a52a0f74ba21e06cd3a03e0cf87072e`. This NXVM-only receiver covers
configure DMA, get FDC binding, configure RTC/CMOS, configure FDC and configure
HDC, including every caller and the callback contexts belonging to these
owners. Existing source authorization and embedded-product exception apply.
No Shared corpus, MyNES, owner INI or external master is in scope.

## Ownership And Similar-Issue Sweep

Construction already publishes the actual borrowed board. The five existing
operations consume it, retaining Core configuration eligibility, route
transactions and unique destruction through the opaque Core contract. RTC
ports, FDC/HDC DMA request callbacks and PIT-to-DMA refresh use board contexts;
cold-reset registration must match construction. FDC's separate Core context
is retained solely for bounded route publication, not board lookup.

No new production function, getter, forwarding wrapper, state copy, registry,
allocation or destructor is planned. Parity/D4 electrical operations, neutral
RAM resizing and its board veto, constructor association and direct-fixture
classification remain subsequent receivers before physical source movement.

## Verification And Delivery

The complete five-operation inventory has 70 baseline occurrences in 24
tracked source/test files: five definitions, five declarations and 60 calls.
All original calls now consume the actual board. Five added null-board cases
bring the inventory to 75 occurrences; they do not introduce another path.
Seven existing controller callbacks consume board context, including the
cold-reset PIT refresh registration. FDC retains its separate opaque Core
connection solely for port registration.

`git diff --numstat -- src/app-nxvm test/app-nxvm` counts 24 paths,
249 added and 212 removed lines, net +37. Production contributes +2 for
explicit null-board guards; the rest is borrowed test handles and receiver
regressions. The existing authority gate gains whole-definition and caller
checks; no new production API, allocation or state owner is added.

The ignored `build/s85-semantic-review.ps1` compares all twelve complete
changed definitions against the S84 baseline. After the explicit Core-to-board
receiver/context substitutions, opaque Core service arguments and new
null-board guards, whitespace-normalized definitions match exactly. This
checks the entire changed function bodies, including failure cleanup and
rollback, rather than only signatures or successful runtime paths. It is
source-equivalence evidence for this finite class, not proof of the remaining
board functions or of completed component extraction.
The same review compares seventeen complete unextended caller files after
normalizing only borrowed board declarations, constructor output arguments
and the five operation receivers. All original assertions match. The two
extended authority fixtures are reviewed separately: they retain previous
assertions and add null-board/status and FDC owner/Core-connection checks.

The x86 tree `build/t540-s4-nxvm-x86-winlibs` rebuilt and passed all 470
repository-only unit cases in 90.21 seconds, plus specialized gates. The
initial CLI argument `-Widths x64,x86` was parsed as one literal string and
selected only that x86 tree: its comma-labelled logs are x86 proof, not
dual-width proof. x64 is consequently run separately with `-Widths x64`.
The copied-source authority probe passed its positive checks and rejected
all 22 injected receiver, declaration, guard and registration violations;
the bounded probe tree was removed afterward.

The first x64 full run completed in 286.60 seconds with 469/470 passing.
`library.kvm_window_modal` failed at its line-74 assertion that the native
modal loop had not yet exited. Its existing registration already requires
`RUN_SERIAL`; the source and test are unchanged. An isolated diagnostic
execution of that exact case then passed in 0.89 seconds without source or
configuration changes. This is an unreproduced desktop-test failure, not
evidence of its cause or permission to skip it. The complete x64 suite was
rerun and passed all 470 cases in 86.04 seconds, including the original modal
case. Its specialized gates then completed successfully, including the
402-row strict-compilation matrix (377 strict, 25 declared deferred). The first failure remains
part of this record; the rerun does not establish its root cause.

All eight optimized Release products have rebuilt and pass PE width, 0.5.0540
banner, absence of compiler-debug sections and freshness against every changed
production source/header. All eight independent neutral executions and all
eight unchanged-INI boots pass. Each boot ran exactly once under its
180-second containment budget: default reached the DOS prompt, XT, AT and
Model 40 reached the running installer. This is checkpoint evidence, not a
claim that intermittent faults can never recur. All owned build/test commands
are terminal. Copied negative probes are removed; ignored caches and scripts
remain needed by the next receiver.

Executor self-review covers all five production paths, nineteen caller tests,
the existing gate and task documents. Core configuration eligibility, RTC
checksum/defaults, DMA binding, HDC route rollback and reset registrations
retain their original behavior. The gate rejects Core receivers and incorrect
callback owners without relaxing prior checks. Documentation governance,
468 changed-document links and diff checks pass. Shared six corpora, MyNES,
owner INIs and external masters are unchanged. Coordinator committed-diff
review is recorded below; this controller boundary does not claim physical
extraction or close T540.

## Coordinator Acceptance

Coordinator actual-change review accepts immediately pushed P1
`8b67d2cdbbfc82125e25b307621d212779e41c33`. Its 38 committed paths comprise
five production files, nineteen caller tests, one existing gate, five task
documents and eight product EXEs. Complete function-body comparison preserves
all twelve definitions after the declared receiver substitutions; seventeen
whole unextended callers preserve their assertions. Actual review of both
extended fixtures confirms only the declared null-board and owner regressions.
All 38 normalized Git blobs match the reviewed worktree, including artifacts;
no Shared, MyNES or owner INI delta exists. Required verification and the
initial unreproduced modal-test failure remain recorded above. Governance P2
accepts S85 only; it changes no executable input and requires no new build.
T540 remains open for the remaining electrical/RAM-veto class, constructor and
direct-fixture association removal, and real neutral Core/flat IBM-PC source
relocation. None is accepted by this controller delivery.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1336568 | `90E1716963E39B2FB78BE89060CEAF45E1618758DEFAE4EB353CBBAD7C49F36D` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1506888 | `B2EC1F5FE999B66D209A5610146F2F520D9F42C1F44364D8670DC6AC086A6EFC` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1352885 | `73FD5A8CE62559AA96D38ED2F614581774DECA215DDC0C66F2B3229A22C11A7C` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1523203 | `675BF44DDA4461A5227450EFC294BC52CF56FD0CD60695F6EBA197258209B963` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1352853 | `B734C5400B484971506DA6DCE2339AF4765F0FD6C5ABE52B8004602503E6D156` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1523170 | `33FB96F287F7F5E422F96063B8D0B023E395DB2264E935FB09698E9A57D53C8D` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1352919 | `BB51D201BC73BBEBD02B3A8A9FC5BB621BA74A3710CC43C065088385EF78898B` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1523238 | `1DFE3AF30BCC75558CB42C216216E42199643F6B2791A5A372072A6959F908C7` |
