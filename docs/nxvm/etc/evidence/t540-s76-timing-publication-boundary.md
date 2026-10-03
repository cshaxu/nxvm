# T540 S76: timing declaration publication boundary

Baseline: S75 P2 `e0f0470ba`. Coordinator admits the full timing-declaration
publication/qualification group; executor confirms the sixteen-field packet.

## Complete receiver and ownership

Production sweep over tracked NXVM source, tests and build descriptions:
`rg -n 'timing_declarations|board_deadline_provider|board_deadline_observe'`.
Plan has two private publication sites (array assignment and copied marker).
Deadline has three private qualification reads. Core getters/scheduler and
same-owner fixtures are valid private-state consumers, not board bypasses.

Core receives a complete copied batch through one construction-only operation.
Its reusable neutral validation checks representation (full count, enum ranges
and unique capability IDs) before any live write. Board plan retains its
different seam/provenance/controller-policy checks; they do not move into Core.
Publication is once-only, rejects closed construction or active firmware, and
retains no caller pointer. Topology and declaration failure destroy the same
unpublished candidate. No partial table or marker becomes visible.

Core passes qualification as a copied value with the existing synchronous
deadline invocation. Board uses it for all three decisions without another
query, callback, state bit or time source. Callback replacement fixtures must
receive and forward the same value.

One Port-B Running-time read belongs to the distinct S77 I/O-cycle contract.
The existing paused-time observation cannot substitute for it. S76 neither
relaxes that restriction nor claims physical relocation is complete.

## Required proof

Extend existing synthetic neutral/plan/scheduler fixtures, full dual-width
units and specialized gates, eight product/neutral builds, each real INI/overlay
boot once, artifact identity and source/test count. Final results follow below.
Shared, MyNES, owner INIs and external master inputs must remain unchanged.

## Implementation and interim review

Both live writes move from plan to the one Core publication operation; all
three deadline reads become uses of the copied callback argument. Neutral
representation validation owns full count, capability bounds/uniqueness and
disposition/seam enum bounds. Exactly thirty unique IDs in the thirty-ID domain
prove completeness without a second coverage loop. Plan preflight reuses that
validation and retains only its different board seam/provenance decisions.
The old duplicate disposition/seen checks and direct write loop are deleted.

Existing neutral linkage executes representation negatives with the invalid
entry last, proving no earlier live entries are published. Null and short/long
counts, duplicate and out-of-range IDs, both enum-bound directions, active
firmware, repeat/frozen publication, reversed input order, input-copy isolation
and reset preservation are exercised. No board archive is linked to this test.
Scheduler checks copied false/true delivery; a programmed XT PIT board callback
uses the supplied value even when Core's private publication marker is false.
The existing whole-plan corpus/provenance/rollback and L2 deadline markers pass.

The authority gate scans all NXVM production .c files, allowing the neutral
machine/scheduler owners only. Injected accesses through `other`, a dereferenced
output pointer and `machine` in a different product source are each rejected;
the positive fixture passes and the generated copy is removed. Marker:
`S76 complete-production timing owner proof: OK`. The retained board-deadline
boundary gate passes. The post-fix sweep finds only neutral state owners and
public operation declarations/calls, not board field access.

Initial development invocations guessed the scheduler target without its Core
prefix and removed a loop variable from the wrong adjacent function. Correct
registered target selection and explicit function-local patch context resolve
both before passing executions. An initially misplaced frozen-negative fixture
was moved after the copy/reset assertions; it was a test ordering error, not a
production publication failure. None of those failed invocations count as proof.

## Diff and executor review

Seven production paths add 60/remove 32 lines (net +28); two existing test
paths add 133/remove 4 (net +129); the existing authority gate adds four lines.
Combined source/test/gate delta is +197/-36, net +161. Documentation and eight
binary replacements are separate. The added production code provides real
full-batch validation/publication, not a forwarding facade or another state
owner. Deleted plan logic was duplicate representation checking/publication;
its distinct source and seam policies remain in place.

The executor reviewed each source/test/gate hunk against S75. Enum domains are
contiguous; invalid capability short-circuits before indexing. Construction
validation precedes every live write; successful copy retains no input pointer.
The unpublished candidate still has one topology/publication failure cleanup.
All deadline qualification decisions retain their previous expressions and
ordering, with the same Core-owned value now passed explicitly. CPU/chip
algorithms and L1/L2/L3 classifications are unchanged. Actual pushed-diff
coordinator review remains required before acceptance.

## Verification record

Final full unit suites pass 470/470 on x64 in 59.55 seconds and 470/470 on x86
in 62.76 seconds. The first x64 full
run had one failure: `library.kvm_window_modal` observed its modal loop already
exited. An isolated diagnostic run and the final full suite both pass unchanged.
The failure's cause is not established; no Shared test/implementation was
modified and the failed run is not counted as a passing suite. Both-width
specialized gate builds complete with exit zero. Documentation governance
passes with the explicit repository-root parameter; the earlier invocation
without that mandatory parameter is not counted as proof.

The production sweep leaves one distinct Running-time observation in Port-B;
S77 must deliver it as part of the actual I/O-cycle input, not a relaxed
paused-only query or mirrored clock. Attachment ownership, direct-test
classification and physical Core/IBM-PC relocation remain T540 exit gaps.

## Artifact identity

Build input is S75 P2 `e0f0470ba` plus the complete S76 source diff. Product
revision remains 0.5.0540. All eight deployed products pass their optimized
Release build check, `objdump -f` expected PE architecture and `objdump -h`
absence of `.debug` sections. They remain beside unchanged owner INIs under
`assets/nxvm/<profile>/`; SHA-256 values are:

| Product file | SHA-256 |
| --- | --- |
| `nxvm_default_0_5_0540_x64.exe` | `B85BF70FC3160369B274CCA9FD905E775F7DE1D0574A708600090C6D9F1A0126` |
| `nxvm_default_0_5_0540_x86.exe` | `EF0E123A39BB26531AA14C35A0DA46E43A3F34C3AE54CA4663E18089DE55D589` |
| `nxvm_xt_0_5_0540_x64.exe` | `A709D2B9E45E01BE539364AC10A51A891274B5349629A8437662E6A772CBEA74` |
| `nxvm_xt_0_5_0540_x86.exe` | `FD09229104E56CCD529A3FD5E0F6D4244D3EB723EC5035AD49956BDE504CB23A` |
| `nxvm_at_0_5_0540_x64.exe` | `E23F93BEE2F37620B20016051BE3129344FFBDF1B0626E660956C128F0986A9F` |
| `nxvm_at_0_5_0540_x86.exe` | `41CFC816A27A9AF90F05012A9AA6E1D629B9B2BCF61092966985B04FB8E27AEF` |
| `nxvm_model40_0_5_0540_x64.exe` | `FBB6757065FE77FD91D933F4432082C814B41E4C148AD36992E775C5AF70F07C` |
| `nxvm_model40_0_5_0540_x86.exe` | `AD372B7A8412A2B1F3F7B275E4DC6606836F63A86DA36175B6F5B13ED2B86DDE` |

## Complete product proof and scope

The serial eight-product process completes with exit zero. Every current
product tree builds the optimized EXE, its real-INI boot probe and actual-source
neutral proof; all eight neutral executions return
`M5:T540:S69:NEUTRAL-LINK:OK`. One external INI/overlay boot per product returns
exit zero: default x64/x86 reach `dos-prompt`; XT, AT and Model-40 x64/x86 each
reach `installer-running`. No boot was repeated or supplied synthetic keys.
These are real firmware/media headless checkpoints, not native UI/audio or
intermittent-fault-frequency claims. Existing live build sessions were observed
to terminal completion, not restarted after observation timeouts.

Shared six corpora, MyNES code/tests/artifacts, owner INIs and external master
inputs have no diff. Local scripts, logs and incremental build trees remain
ignored. The complete executor delivery is ready for P1 push and subsequent
coordinator review of the actual committed diff; S76 is not yet accepted here.
