# T545 S7 Final Receiving Qualification

## Frozen Requirement Map

Baseline is 43e9700e6. Owner requests complete T545 closure before CPU repair.
The original [proposal](../../proposals/m5-softpc-eight-corpus-refresh.md) and
[history](../../history/M5-T545-softpc-eight-corpus-refresh.md) retain scope.
This is final receiving qualification, not CPU instruction/manual qualification.

| Requirement | Direct proof and disposition |
| --- | --- |
| Accepted fixed source/import | S1 inventory and S2 8124e551e841ccdec2ceb7f6a0f6ae5b513a7951 exact eight-root import and receiver map remain immutable historical evidence. |
| Subsequent owner-local test revision | S3-S6 evidence and target-correct commits map each contract/assertion relocation; current bytes are not called identical to the old SoftPC pin. |
| Eight current manifests | Independently run src/common/verify_manifest.cmake for src/test of Lib/Common/x86/IBMPC; all eight pass. |
| Public ownership and dependency boundaries | Re-read actual production diff against 826eccc93: one Audio failure/worker owner, locked Common frame validity, deleted empty Core bus wrappers, embedded IBMPC atomics, slot arrays and failure-preserving Product cleanup. Applicable corpus/Types/DAG/negative checks remain required. |
| Complete PC/shared units | Fresh complete 532/532 pass per width: x64 200.57s; x86 68.84s. No failed/skipped case. |
| MyNES receiving units/integration | All registered 43 product units pass per width (23.03s x64, 35.47s x86); all 12 integrations pass per width (27.84s x64, 33.87s x86), including native desktop cases. MyNES source/configuration/assets unchanged. |
| Original PC integration universe | All 58 pass once with unchanged predicates and real fixed bindings; detailed matrix below. No skipped case or weakened checkpoint. |
| Firmware/media path | Existing compiled firmware and adjacent INI load through sole Product factory; session_ini.c directly opens external media without copy/path rewrite. All declared media modes are overlay. |
| Current product artifacts | All ten hashes match S2 evidence; x64 PE=8664 and x86 PE=014C; no .debug/.zdebug/.stab compiler sections. No executable inputs changed in S3-S7, so no manufactured rebuild/version or binary diff. |
| Governance/actual review | Active sixteen-field packet, allocation, queue/proposal/archive links and final actual-change review required; initial documentation gate and diff check pass. |

## Execution And Containment

Reused ignored receiving roots t533-s5-default/at/model40/xt-release-x64
and corresponding x86-msys roots. MyNES uses t537-s7-mynes-make and
t545-s2-mynes-x86-clean. Incremental builds prepare current test binaries;
CTest executes registered cases, not build-only coverage. Eight jobs for units
and non-desktop gates; native desktop integration runs sequentially.
Original 58 PC contexts run once, not three times per group.

Initial x64/x86 unit aggregate builds were intentionally interrupted before
CTest execution to separate native desktop ownership from AT integration;
compiled objects were retained. This was an explicit scheduling decision,
not an observation timeout, failure or restarted unknown-live process.
Both final complete unit runs end successfully. An additional x86 integration
build was explicitly interrupted before tests to avoid concurrent writers in
the same receiving tree; its later sole aggregate completes successfully.

The external input identities below match before and after integration,
proving overlay tests did not modify masters. Files remain
in the owner-managed external media-nxvm archive, never repository payloads.

| Input | SHA-256 |
| --- | --- |
| fdd_1440k_msdos_500.img | FADEB3A27C6A0E1CF582DDE0B9AECB7E5D30678F2F967F2F4562F167CC0CB1D5 |
| fdd_1200k_msdos_500a_01.img | 0F51D92B482253FC468A2B470FFAB82DB43898D1C8B44E504808B7A3EF3D4BDE |
| fdd_360k_msdos_500.img | DE271368874209C07A2FC25C81C17529D4BDD7718B2731B86C49A2DA923A256E |
| hdd_50m_win31.img | 61E5CDC0B76151CC65B73EB44094738B9DE86052B1B07F20FC03205984CD77E1 |
| hdd_40m_deskpro_386_blank.img | 2BBC68E612A72290A5181E070494A7580CBECE1F528F92F009A269DC05819E73 |

## Boundary

T544 CPU defects and the separate queued CPU repair remain intact. Existing
TODO entries (Common wake failure, native desktop isolation, Console rollback,
physical timing and broader hardware) are not claimed repaired by an import
or passing boot. No production/API/INI/asset change is required.

## Final Runtime Matrix

| Fixed product | x64 | x86 |
| --- | --- | --- |
| NXVM/default | 22/22, 13.00s | 22/22, 15.10s |
| My5170/AT | 3/3, 40.20s | 3/3, 48.92s |
| MyDeskPro386/Model40 | 3/3, 64.01s | 3/3, 84.29s |
| My5160/XT | 1/1, 22.37s | 1/1, 27.16s |

## Final Static Receiver Corrections And Review

The specialized aggregate exposed two omitted S5 receivers, not new CPU or
hardware defects. T332 still demanded the former board fixture for the neutral
FPU S65 case. It now checks its real neutral constructor, explicit bind/freeze/
reset/debug bootstrap and local debug/exception fixtures, while rejecting
IBMPC/private CPU setup. T344 still listed two relocated sources under old
IBMPC paths; four entries now follow their x86 owners. The 44-owner lifecycle
inventory and 133 constructor classifications are preserved, not reduced.
Similar-issue review follows all five S5 relocations and receiving registrations;
the three decoder paths were already correct. No Shared tree was patched.

Counted tracked NXVM CMake changes: two paths, +22/-4, net +18, using Git
numstat excluding evidence/manifests/assets. The increase owns the missing
neutral fixture admission predicate, not a production framework or forwarding
layer. No executable input changes: src is byte-identical to S2 Shared P1
fc3c73aa1, all ten artifacts keep their S2 hashes and width/debug checks.

All eight manifests pass. Supplemental corpus/Types/DAG/ownership/negative
CTest checks pass 33/33 per width (225.79s x64, 325.06s x86). The slow Types
layout self-test is not a runtime/CPU timing result. The corrected x64 full
specialized aggregate plus artifact-root and integration-INI boundary gates
pass on both widths. Its expected negative self-test emits an error for the
injected duplicate ownership row, then asserts that rejection and exits zero;
this is not an ignored failing check. Documentation governance and diff checks
pass. Every owned build/test handle is terminal; receiving caches are retained
for the immediately queued CPU repair's incremental regression verification.

Executor self-review maps the complete original request to the frozen table,
checks every new checker predicate and all path substitutions, verifies the
full original runtime matrix and rechecks assets. Applicable architecture and
coding skills confirm one owner and no parallel path; they drove the receiving
checker correction rather than a Shared implementation patch. Source/build/test
predicate and artifact identities remain preserved. Implementation delivery is
ready for coordinator actual-change review; no T closure is asserted here.
