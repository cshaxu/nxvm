# T540 S96 Artifact And Build-Cost Evidence

Owner: NXVM; purpose: current eight-product verification and the owner's build/
test performance request. Current owns admission and acceptance; S97 owns the
complete pending implementation delivery. This record does not close T540.

## Build-Cost Repair

The retained Release trees use MinGW Makefiles. The first default x64 command
built the product and integration probe in sequence. CMake generated twice:
79.6 seconds before the product and 59.3 seconds before the probe. These are
observed generate times, not compiler times or sustained benchmarks.

The deployment script read the selected tracked owner INI, then wrote those
bytes back to that same path after linking. That INI is a configure dependency
through `configure_file(... COPYONLY)`. The write changed its timestamp and
invalidated the next target's build-system check without changing its content.

Delete this write-back and its unused argument at the single NXVM deployment
owner. Deployment now copies only the EXE. Four profiles and both widths use
that same owner. No runtime source, shared corpus, compiler option, test
assertion, firmware, media, owner INI or MyNES file changes. Do not suppress
CMake regeneration, bypass dependency checks, add a successful-result cache,
change generators underneath existing objects, or rebuild MyNES.

The existing artifact-roots gate now rejects `file(WRITE/APPEND ...)` in this
EXE-only deployment script. A temporary ignored copied-authority negative with
the old INI write restored fails with the intended deployment diagnostic.
The actual current gate passes. Direct deployment twice verifies byte-identical
EXE deployment and unchanged hashes and timestamps for all four owner INIs.

After the CMake argument change, the current default x64 tree regenerated once
(49.3 seconds) and relinked its product. The subsequent probe target did not
regenerate. This removes the redundant trigger; it does not eliminate the
cost of legitimate configuration changes or Make's dependency traversal.

A subsequent unchanged-input two-target build completes in 34.640535 seconds,
with no configure/generate or compilation/link step. This confirms removal of
the invalidation loop, while exposing the remaining Make traversal overhead;
it is not evidence that all S work becomes instantaneous. Keep existing object
trees for this finite eight-product verification instead of discarding them
for a cold generator migration.

The narrow repair changes two deployment/caller files by deleting seven lines;
the existing static gate gains five lines. Review the whole deployment body,
the actual `add_current_vm_artifact` caller and gate, including PE rejection,
copy failure propagation and `ONLY_IF_DIFFERENT`. Those behaviors are unchanged.
S94's inventory is its frozen review baseline; this S96 follow-up owns these
current differing build-script identities:

- `NxvmProduct.cmake`: `C7AF2D2D47698ECEB54668AA356C11123AE32F4312AC1F94937BF033B24F56C4`
- `deploy_current_artifact.cmake`: `9F9056491F5A64D7DDE4253DAC9B65A68EFEC620E285B67B32BA694BB7F0DE92`
- `verify_product_artifact_roots.cmake`: `08765C3E6521F5590C15D699C1034A3ECA78C0C3AB8ED663640F7A5E6E2EE6F2`

Earlier S94 optimizations remain: strict-matrix command batching measured
36.140738 to 4.9107794 seconds; CPU manifest indexing measured 4.388 to 2.012
seconds. Root complete units and independent tools-on/off suites remain their
recorded actual S94/S95 runs. Do not rerun identical runtime suites for this
deployment-only change or a documentary reading batch. Required eight boot
groups run once each and full T-level integration remains an S97 requirement.

## Current Product Verification

The current default x64 product and probe build succeeds with `-O3 -DNDEBUG`,
runtime-development tracing disabled and `--strip-debug`. Its first unchanged
owner-INI probe exits zero with `dos-prompt` in 4.38 seconds. The deployment-only
relink changes no runtime input, so this result is not repeated.

All eight artifact builds and once-only boot checkpoints complete below. This
accepts S96 verification only, not implementation delivery or T closure.

Default x86 completes its build in 153.2581564 seconds, including one legitimate
84.8-second generation for the changed CMake graph. The second target does not
regenerate; its unchanged-INI boot reaches `dos-prompt` once in 3.2060803 seconds.
Both widths' real Core flags are `-O3 -DNDEBUG` with
`CORE_MACHINE_RUNTIME_TRACE_ENABLED=0`. The affected registered artifact-roots
gate passes in both retained unit trees; each Ninja configuration regenerates
in about 4.6 seconds before the gate. This cross-generator observation is not
an equivalent cold-build benchmark.

One diagnostic command used a nonexistent historical unit-tree name and exited
before configuration or test execution. Corrected commands use the actual
`t540-s8-unit-x64/x86` trees and pass. Documentation governance and diff checks
also pass. No unchanged complete runtime unit suite is rerun for this
deployment-only repair.

XT x64 completes its build in 129.6356806 seconds, including one 72.5-second
generation. Its once-only unchanged-INI boot reaches `installer-running` in
19.1520058 seconds, exit zero. No second generation occurs between the two
build targets.

XT x86 completes its build in 123.770151 seconds with one 80.1-second
generation. Its once-only unchanged-INI boot reaches `installer-running` in
22.0533658 seconds, exit zero. Four of eight required boot groups now pass;
AT and Model40 verification remains pending.

AT x64 completes its build in 118.347105 seconds, including one 69.8-second
generation. Its once-only unchanged-INI boot reaches `installer-running` in
29.5726296 seconds, exit zero; no keyboard POST failure is observed by the probe.

AT x86 completes its build in 106.6819659 seconds, including one 62.5-second
generation. Its once-only unchanged-INI boot reaches `installer-running` in
38.5526249 seconds, exit zero. Six groups pass; only the two Model40 groups
remain pending at this progress node.

Model40 x64 completes its build in 111.7201292 seconds, including one
65.7-second generation. Its once-only unchanged-INI boot reaches
`installer-running` in 50.2806294 seconds, exit zero. Seven groups pass.

Model40 x86 completes its build in 117.0905733 seconds, including one
66.1-second generation. Its once-only unchanged-INI boot reaches
`installer-running` in 68.5604164 seconds, exit zero. All eight groups pass
without a repeated boot, synthetic input or configuration/media substitution.

## Eight Artifact Identities

All eight real Release caches select their recorded profile. Actual Core
compiler flags contain `-O3 -DNDEBUG` and runtime-development trace disabled.
Each product has the expected PE width, banner `0.5.0540`, no `.debug` section,
exactly one production Core archive and defined neutral Core constructor, and
byte-identical build/deployed EXEs. The runtime debugger remains included.
Each file below resides in its existing `assets/nxvm/<profile>/` directory.

| Product file | Bytes | SHA-256 |
| --- | --- | --- |
| nxvm_default_0_5_0540_x64.exe | 1362854 | 486033D7D56167B4E89E3F1917E69CBAEFC79124EC9BFF81B46CC98C6902231E |
| nxvm_default_0_5_0540_x86.exe | 1533592 | F97A6DBB07316998486B9A5C491C8C188B9793001F19B6DE215B17FEA0DB55B6 |
| nxvm_xt_0_5_0540_x64.exe | 1362822 | 9ADC7A20D8C1EA1D7CD344EE40BC0FAC5A7D603BAC980FE97C49959003E9CCAD |
| nxvm_xt_0_5_0540_x86.exe | 1533559 | E4C93C8291D20D725847EC22E7EE4D896E4843BE07DB4118B9AF6EC3FAA0E18C |
| nxvm_at_0_5_0540_x64.exe | 1362888 | 28DF2770EEB479C0E8492D4F33ED39D7A7407A7993BE65EEF20229F883C9099A |
| nxvm_at_0_5_0540_x86.exe | 1533627 | 8F8FBFB10BEF7C3C2466DF17F3C461CD46DBB5664B58EAFCE3658F516B3F0BE7 |
| nxvm_model40_0_5_0540_x64.exe | 1346537 | 0D911148D7D6BA74E5E651FBC32804A04B6861C247DE2F8C955275E0C914C35F |
| nxvm_model40_0_5_0540_x86.exe | 1517277 | D4CEFE8755D19142A3F3183EE00332188E70F0371EE0499CAF78B83858B840E1 |

## Coordinator Result Review

Review the actual deployment/caller/gate change, the completed command exits,
eight identity checks, eight precise terminals and unchanged owner input hashes.
All owned builds/probes exit; no boot is retried. The isolated negative copy
is removed after its intended rejection. Retained ignored build/proof scripts
remain inputs for immediately following S97 delivery review.

MyNES scoped Git status is empty and its two binaries retain the starting
hashes. Four owner INI hashes remain exact. The six shared manifest bodies and
runtime/test source remain the S94/S95 inputs; the deployment-only follow-up
does not invalidate their full runtime suites. Both affected registered gates,
documentation governance and diff checks pass. S96's artifact/verification
result is accepted without partial P. S97 still requires final whole-tree
reconciliation, external integration, scoped complete delivery and T audit.

## Owner INI Identities

- Model40: `4C363DB4FD3FA8864C89B2557FA2A2BF93994E67E295B11AEE07053EC1FDB0E3`
- Default: `A858BEE0D2145E539D69861A675FDDB10433B25EAD658A9DFAF0D69632989F13`
- XT: `48900ED858BE7B6A5AE8CD452B4F4432FC9446365FC366A49470DA331C443B29`
- AT: `282478D7DFB479070FDB138867DD08C99C5283770C5872DDEA07529D32C8A15C`

Runtime media retain the owner INIs' overlay access. External masters and
MyNES binaries are not rebuilt or modified.
