# T540 S82 Configuration Board Publication

## Scope And Confirmation

The executor confirms CURRENT's S82 packet against accepted S81 P2
`a5222073b`. This NXVM-only receiver completes the remaining constructor
publication class, not public board operations or physical Shared relocation.
The existing source provenance and owner-approved embedded-artifact exception
remain unchanged. No Shared/MyNES source, test, configuration, artifact or
external asset master is modified.

## Actual Boundary And Complete Caller Sweep

The existing public configuration constructor and its two private RAM/port
allocation-failure seams accept an optional opaque board output. They forward
it to the existing sole candidate factory. That factory already clears every
supplied output before validation and publishes only after successful board
construction. The board is the actual allocation installed as attachment
context, borrows its opaque Core handle, and is released only by Core's
existing attachment finalizer. No function, allocation, getter, registry,
destructor or alternative lifecycle path is added.

`rg` over NXVM source/tests finds 198 existing public configuration calls in
125 test files and five allocation-seam calls. All adopt the new signature;
the added three-topology publication loop adds one textual call, producing
204 total test calls. An arity sweep finds zero old signatures. MyNES's
different implementation is excluded. Plan construction was already handled
by S81 and is not changed here.

All 123 test files outside the two deliberately extended RAM/port regressions
were compared against the actual baseline blob plus precisely the one optional
argument substitution: every file is otherwise identical. No instruction,
assertion, fixture ordering or expected hardware result changed. Pure execution
fixtures pass null rather than retain unused board state. The direct field
fixture and board-operation class still needs migration using the output;
this receiver does not pretend that private Core association has disappeared.

## Verification

The existing RAM test checks publication identity across default, auxiliary
PIT and XT configurations; both successful memory sizes, both allocation
failures, all seven invalid ratio preflights, null configuration and missing
Core output. The existing port test checks all three configurations and every
injected port-allocation failure, plus the successful boundary after the last
allocation. Requested board outputs must be null after each failure.
Both tests passed and retained their original markers; the added marker is
`M5:T540:S82:CONFIG-BOARD-PUBLICATION:OK`.

The controller-authority gate checks each existing factory's output forwarding.
A positive copied-source probe and twelve injected negative variants pass,
including a dropped board output at each of the three factories. Probe sources
are bounded below build, never tracked production mutations.

Complete unit runs retain all 470 entries: x64 470/470 in 285.46 seconds,
x86 470/470 in 88.91 seconds. The existing serial CPU boundary negative
passes in both suites; no failure or retry is hidden. x64 specialized gates
also pass; the x86 specialized gates likewise pass, including both 402-row
matrices (377 strict, 25 explicit deferred). Both complete build/test/gate
scripts exit successfully. The documentation gate, 468 local links and
`git diff --check` pass.

All four existing product builds, both widths, rebuild the current 0540 EXE,
boot probe and independent neutral Core proof. Each neutral executable passes;
each unchanged INI/external-media boot runs exactly once. Default x64/x86 reach
the DOS prompt; XT, AT and Model 40 x64/x86 reach the running installer.
The 180-second limit is containment, not a success checkpoint. These are eight
single samples, not a claim that intermittent hardware faults can never recur.

All eight deployed products pass PE width, 0.5.0540 banner, source freshness
and absence-of-compiler-debug-section inspection. Release optimization also
passes each build's artifact gate. INIs, external masters, MyNES's 0043 pair
and all six Shared source/test corpora have zero diff.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1336056 | `85E47C1C2E12C3439A01950677D542428E251F5F6A5FC4056C0B68E95DAD9B78` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1506888 | `45FDC0364515A41E88DBDFCC6FD543859ABE554329CD89120A518412EF23B5C2` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1352373 | `F7D18FC4F0F5F8ECFE2F6B4C4079AEA418955613F4CC4B8B4577A0F5BD55BA25` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1523203 | `7C7452E7B504C52E1AD03E9D1490DC611795279516D830B42582D189538ABFBA` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1352341 | `2E7EFDED22B891793C566BC1B6F56D5E61CE5FB7AD32399132670F47670180F7` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1523170 | `63A9A3138C1B0F098C189B1801EF28705F2C3C6CFC4BD3D9F2348E2B7E25DF3F` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1352407 | `22C01F75B4F49FD9414CC6A911294292E8C2BBA005A3AB1434C208CFF4CF5762` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1523238 | `6AC630AEE5C3AEC12D863DD90E0EC3B7913DD51D2BB055F5EAA8F31415677C7C` |

## Size And Retained Owners

Source/tests: 128 paths, +263/-217, net +46. Production alone is three paths,
+15/-9, net +6. Most file count is the complete 203-call signature migration;
positive line growth is the publication/failure regression, not a wrapper or
second implementation. The existing gate adds eleven lines.

The next receiver remains the public board-operation/direct-fixture class,
followed by private-association deletion and actual neutral/IBM-PC relocation.
Core still uniquely owns CPU/memory/port/time and finalization. Board state
still uniquely owns wiring. T540 remains open.

## Coordinator Acceptance

Implementation P1 `37b941c35` was immediately pushed to origin/master before
this review. The coordinator inspected its complete 142-path committed
inventory and actual source, regression, gate and documentation diffs. All
123 mechanical caller files match the prior blob plus exactly the optional
argument change; both nonmechanical regressions preserve the old checks.
All eight committed EXE blobs equal the verified worktree artifacts. Output
lifetime, failure clearing, sole allocation/finalization and actual verification
agree with the packet and ledger. No out-of-target or INI change is present.
S82 is accepted; only its active packet is removed. No T closure or new
implementation scope is claimed by this governance-only acceptance.
