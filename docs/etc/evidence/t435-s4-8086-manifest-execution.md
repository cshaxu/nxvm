# T435 S4: 8086 Manifest Execution

## Scope

This evidence indexes the actual 8086 execution result for the originally
frozen T435 S2 universe. It covers its 649-key inventory: 244 base rows, 178
legal context rows, and 227 legal combination rows. The artifact is
`docs/etc/cpu-timing/t435-s4-8086-timing-results.json`.

## Result contract

`core-machine-8086-timing-manifest-runner` executes each key using a real
8086 Core machine and its retirement observer. It rejects a wrong tick,
functional side effect, missing required input, wrong origin, unclassified
retirement, missing source form, or missing key observation. It writes the
result artifact only after every recipe and coverage check succeeds.

`Verify-8086TimingResults.ps1` consumes the frozen canonical manifest and
rejects a missing, duplicate, unknown, failed, unallocated, or provenance-
mismatched result. It also requires the published formula-input bits implied
by each LOCK, segment-override, odd-word, and REP/REP-phase key.

## Executed result

The generated artifact contains exactly 649 unique records:

- 617 L3 records: 228 base, 166 context, and 223 combination.
- 32 named `L2:G3` records: 16 base, 12 context, and 4 combination.
- 76 observed LOCK, 228 segment override, 218 odd word, and 132 REP plus
  repeat-phase input observations.
- Zero `source_timing_unallocated` records.

The runner reports `M5:T435:S4:I86-MANIFEST-PROBE:PASS:649/649`; the verifier
reports `8086 timing results verified: conforming_keys=649`.

## Source-form repair

The result verifier exposed a genuine route gap during S4: the string/REP
calculator returned its ledger tick but did not identify its selected source
form. `core_machine_string_io_source_instruction_cost()` now publishes that
form before producing a successful result. This preserves the string/REP
origin and tick calculation while making every successful string retirement
traceable through the shared timing publication seam.

## Build and regression evidence

The project configured from scratch with the `MinGW Makefiles` generator in
`build/t435-s4-make`; this alternate generator was used because the host's
Ninja process launcher could parse build files but could not start child
commands. The current `vm-0-5-0434` target built successfully and published
`build/output/nxvm_0_5_0434.exe` (SHA-256
`DC48A7BDADF4942DA726FE17DFF076A0AC8C945149D93FCF30D28200E29DC4D9`).

The complete `current-gate` CTest set passed 291/291 after updating two
historical timing smokes to their S4 ledger values: the 8086 `AAA` row is the
manual's exact four ticks, and the 8086 Group-3 cases use their named
`L2:G3` values (rather than superseded pre-S4 approximations). The focused
set also passed 6/6: retirement observation, the two repaired historical
smokes, the 8086 ledger smoke, the 649-key runner, and the result verifier.

The four-profile manifest contract passed at 3,295 canonical keys, and the
T359, T360, T435 S3, T388, dependency-DAG, C-facade, documentation-governance
and whitespace gates passed. This evidence still requires the coordinator's
actual-change review and P commit/push before S4 acceptance.

## Later scope correction

T435 S5 established that this historical freeze omitted the successfully
decoded `XLAT` (`D7`) instruction. Therefore the statements above are limited
to the 649 keys actually frozen and executed by S4; they are not a claim of a
complete 8086 decoder corpus. S5 adds `I86-XLAT` (11 clocks) and its legal
`I86-XLAT-SEGMENT` context (13 clocks), and publishes the superseding 651-key
runtime result artifact `docs/etc/cpu-timing/t435-s5-8086-timing-results.json`.

## Change accounting and retired paths

Git's staged `--numstat` reports production source changes of +489/-37 lines
(net +452). Test source changes are +3,600/-32 lines (net +3,568), including
the 3,071-line exhaustive runner; its explicit per-key recipes are the reviewable
proof of the finite 649-key contract, not a second production selector. The
test-only result verifier is 72 lines and the generated result artifact is
655 lines. The larger test delta is therefore intentional, bounded to the
frozen manifest, and does not expand the public ABI.

The former 8086 successful-retirement fallback origin is gone. The retained
8086 selector is now the `PRIMARY` source owner; string/REP forms publish a
source form before success, and `source_timing_unallocated` remains a rejected
successful state. The two historical smokes no longer duplicate a superseded
AAA or Group-3 timing model: they assert the S4 ledger's exact/manual or named
L2 values through the shared publication seam.

## Coordinator acceptance

After P1 `a2d66c19` was pushed, the coordinator reviewed the active S4 packet,
the original owner request, the staged implementation and generated artifact.
The review confirmed that all 649 frozen keys map to observed successful Core
retirements; that the L3/L2 provenance, required inputs and source forms are
validated; that no public ABI or board-timing scope entered the patch; and that
the prior 8086 fallback did not survive as a competing production route. The
full current-gate result is 291/291 passing. S4 is accepted. T435 remains open
only for its later profile batches. T435 S5 subsequently corrected the omitted
`XLAT` keys as documented above.
