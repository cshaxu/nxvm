# T540 S75: lifecycle observation and READY ownership

Baseline: accepted S74 P3 `9221e17dc`. Coordinator admits the complete
six-operation board lifecycle consumer group and both neutral READY
definitions. Executor confirms the Current sixteen-field packet.

## Source decision and sweep

`rg -n '(machine|mutable_machine)->(?!board)[a-z_0-9]+'` with PCRE2 over
board*.c, machine_board*.c, machine_plan*.c and machine_display.c identifies:

- Keyboard native byte, native bytes, native scan-set, mouse relative, display
  capture and display observation: private lifecycle reads. These six named
  operations are the entire lifecycle consumer group. Use the existing copied
  lifecycle operation, preserve their exact accepted states and status codes.
- Both CPU/DMA READY definitions: neutral transaction gate and mutable signal
  fields. Move verbatim into the existing neutral scheduler; no new signature
  or forwarding function.
- Port-B source-time read and three board-deadline qualification reads: remain
  next S76 intake. `get_elapsed_ticks` rejects Running and Initialized; port I/O
  is an executor-time operation. Replacing it blindly changes behavior. Their
  contract must be resolved before attachment and physical movement.
  The broader arrow-field sweep additionally identifies both plan publication
  sites, `(*out_machine)->timing_declarations[...]` and
  `(*out_machine)->timing_declarations_copied`. They belong to that same complete
  timing-publication/observation receiver; copying a getter alone cannot close
  it. This is retained work, not an already neutralized publication path.
- Callback slots and the one board attachment: retain the separately recorded
  attachment receiver. Neutral Core implementations and same-owner fixtures
  are legitimate private-state users, not board bypasses.

Source reconciliation counts six functions rather than repeated lifecycle
comparisons; the packet's initial seven-count was corrected before implementation.

## Verification and delivery

Existing synthetic fixtures verify independent READY linkage and complete
lifecycle/output guard semantics; the production owner boundary is mechanically
enforced. GUI/audio acceptance and full T integration are not inferred from
headless boots. Source delivery requires immediate P1 push and subsequent
coordinator actual-diff review before acceptance.

## Implementation review

Both READY function bodies are identical to the baseline after whitespace
normalization. Null/mutation/gate failures keep INVALID_ARGUMENT, any nonzero
input normalizes to true, and initialization/cold reset retain true defaults.
The existing actual-source neutral fixture exercises disabled gates, enabled
gates with 0/1/-3, the active firmware mutation guard, freeze/reset and repeated
cold reset. It links no board archive or attachment.

All six lifecycle callers keep their original null/output status and accepted
states. In particular, scan-set still permits Faulted while native input does
not; display still permits only Stopped/Paused; mouse still delegates AT
enablement to the existing controller and returns Unsupported for XT. The
existing input/display fixture now supplies all five lifecycle states with
synthetic XT input, plus every null machine/output guard. Same-owner fixture
setup does not add a production transition or external input.

The existing controller-authority scan covers the entire production board
source family, not just the original file. Four injected violations in another
board source (lifecycle, transaction contract, CPU READY and DMA READY through
the mutable alias) are rejected; the positive fixture passes. Temporary copied
sources are removed. Marker: `S75 lifecycle/READY production boundary proof: OK`.

Initial local invocations exposed a guessed target name, guessed allocation
vocabulary, and a CTest expression missing its registered `unit.` prefix.
They provided no passing proof. The corrected existing targets compile and both
fixtures execute their original markers successfully. Complete repository-only
units pass x64 470/470 in 279.03 seconds and x86 470/470 in 74.06 seconds.
Both complete specialized-gate runs return zero, including 402 timing-matrix
rows (377 strict and 25 explicitly deferred). The deliberate deferred-negative
selftest's expected diagnostic is not a production failure.

Tracked production delta is +47/-30, net +17 across three existing files.
Existing tests are +115/-1, net +114; the authority gate is +13/-0.
Combined code/test/gate delta is +175/-31, net +144. The production increase
is copied local observations and explicit result checks, not another owner,
API, wrapper, queue or execution path. Documentation and binary changes are
reported separately from these source counts.

All eight product/probe build jobs and their independent actual-Core linkage
executions return zero. Each neutral proof emits
`M5:T540:S69:NEUTRAL-LINK:OK`; the last Model-40 x86 job completed the serial
build process normally. Existing processes were observed to completion, not
restarted after an observation timeout. The first XT x86 boot launch was not
executed because permission review timed out; its one allowed launch retry
is the only actual run, not a second boot.

One external INI/overlay boot per deployed product completes with exit zero:
default x64/x86 reach `dos-prompt`; XT, AT and Model-40 x64/x86 each reach
`installer-running`. All eight existing inputs are retained, with no repeats
or synthetic guest key injection. These are real firmware/media headless
checkpoints, not native GUI/audio acceptance or intermittent-fault frequency
claims. Documentation governance and `git diff --check` pass.

Executor self-review confirms all six guards and both verbatim READY bodies,
complete tests/gates, artifact identity and NXVM-only scope. P1 is the complete
delivery; coordinator acceptance follows actual review of its pushed diff.

## Artifact identity

Build input is accepted S74 P3 `9221e17dc` plus the complete S75 diff. Product
revision remains 0.5.0540. All eight deployed products pass the build's optimized
Release check, `objdump -f` PE architecture and `objdump -h` absence of `.debug`
sections. They remain beside their unchanged owner INIs under
`assets/nxvm/<profile>/`. Full SHA-256 values:

| Product file | SHA-256 |
| --- | --- |
| `nxvm_default_0_5_0540_x64.exe` | `60EEB19DB8200B65A012B7ACE31AFC3FD577506B30CB3D6C1755ADD3A6ED73D1` |
| `nxvm_default_0_5_0540_x86.exe` | `313C3DAE5A331A58BB78CBBA4B109B6D357C5D5A33C6752A1A5CC01A3024572C` |
| `nxvm_xt_0_5_0540_x64.exe` | `392B8D5FC66131999F46D738AC9A0444211020202B31E0FDACB44EB8A9D7F165` |
| `nxvm_xt_0_5_0540_x86.exe` | `4B3670FF235557E4A969202C978AB6782CA88239F15068079D482F43B7A6FE25` |
| `nxvm_at_0_5_0540_x64.exe` | `897C2C1413CE455FED9FC005682C80CAB11F11F50104594D624B109064DBD291` |
| `nxvm_at_0_5_0540_x86.exe` | `607E9D4A7AA8D5BDCDE516F5FD5077A56FBA63D3C5F4C9D3F1BFC83B85E909D6` |
| `nxvm_model40_0_5_0540_x64.exe` | `EE37C51BF81B21D2587ED2643E5D4C6CE97A3451263D4EBB7F8A5BF9DC9F3243` |
| `nxvm_model40_0_5_0540_x86.exe` | `CFA12F91767B27E097CC37D12CF4A364F79347E0D626850BAB691FFE1B6D64C9` |

Shared six corpora, MyNES code/tests/artifacts, owner INIs and external master
inputs have no diff. Logs and incremental trees remain ignored local evidence,
not distributable artifacts. Running-time/timing publication, attachment and
physical Core/IBM-PC relocation remain required T540 receivers.
