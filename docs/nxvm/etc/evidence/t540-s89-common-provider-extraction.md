# T540 S89 Common Provider Extraction: Delivery Evidence

Baseline: accepted S88 `bbeb245b418d425e0433c0e9a55cd0fc635b9815`.
Coordinator actual-commit review accepts target-separated Shared P1
`4e23f14b64361e1118b9a8af92dac45ad32c6116` and NXVM P2
`eed6b8e54cea01b2bd07b0f8e612d5c97b370635`.

## Complete Receiving Batch

Four production source/headers physically move from App devices to flat
`src/x86/ibmpc-common`: media_interface.c/h and display.c/display_interface.h.
One original pure media registry test moves to `test/x86/ibmpc-common`.
All live source/test/build consumers use the new public contracts. The Shared
target compiles both implementations exactly once and links only Types;
the display ABI borrows the public video-values header. No file backend,
App selection, native handle, additional state or forwarding function is added.

Mechanical comparison against baseline proves all four production bodies
unchanged except public include paths, LF normalization and deletion of the
unused Core device-support include. The original media test differs only in
its include path; every assertion remains. A direct display test covers empty
slot, invalid arguments, binding, frozen rebinding rejection, borrowed context
updates and release. App hardware/firmware tests remain at their current owner.

Actual diff review against the fixed baseline counts 44 source/test/build/tool
paths with rename detection: +118/-64, net +54 lines. This excludes documentation,
manifests and generated/artifact paths and includes the 44-line new display test.
The positive balance is test and ownership-proof coverage, not new runtime logic;
the two production implementations differ by +2/-3 mechanical lines in total.
The build diff removes the old executor archive, primitives alias and App test
registration; both runtime board variants retain their same chip dependencies.
The C source/header/test-only count is 36 rename-aware paths, +78/-35, net +43;
the new display test accounts for +44. All thirty modified App source/test
files compare equal after only the two public include-path substitutions.

The contract-preserving review also identifies a pre-existing unused mode
notification half of the display slot: App bind supplies mode_context and
mode_provider, but no slot operation dispatches them. Snapshot dispatch is the
live path. S89 preserves the admitted ABI rather than inventing a new mode
callback invocation; the next common-board/provider caller batch must remove
the unused parameter/storage and App callback together. This is explicitly
unaccepted residual cleanup under T540, not a claim of a dead-code-free final
board corpus or a timing/behavior regression introduced by this move.

NXVM's former display-only executor archive and unused primitives alias are
removed; both live board target links name the actual Shared target directly.
No compatibility alias or forwarding archive remains. Board targets explicitly
link their real chips. The direct strict
matrix now names x86-ibmpc-common: both files compile strictly. Display's former
deferred entry is removed rather than retaining a false exception. Five obsolete
App-to-App dependency allowlist edges disappear with their callers.

## Observed Verification

- Independent target and both provider tests build and pass without App source.
- Independent repository-only unit subset: 119/119; remaining four independent
  manifest/corpus/verifier-negative/test-manifest checks: 4/4. Together these
  cover the entire 123-case standalone registration.
- Both changed manifests verify. Shared corpus boundary and original/new
  negative dependency checks pass.
- Six duplicate-provider restoration/compilation negatives reject as required;
  the actual positive controller-authority gate passes.
- NXVM dependency DAG verifies twenty remaining exact migration edges.
- x86 full unit suite passes 471/471 in 69.57 seconds. Initial specialized run
  found stale dependency allowlist and deferred-display classification; both
  were corrected from actual source/compile evidence. The x86 specialized
  rerun passes all 82 checks; its direct matrix has 402 rows, 381 strict and
  21 retained deferred entries. The final 82-check rerun after alias deletion
  also passes.
- x64 full unit suite passes 471/471 in 265.37 seconds. Its complete subsequent
  125-check specialized run exits successfully, including the strict matrix.
- All six source/test manifests verify. All eight product and test-target builds
  complete successfully. The eight EXEs pass PE architecture, 0.5.0540 banner,
  absent debug sections and input-freshness inspection.
- The completed default x64/x86 products each reach the original DOS prompt
  checkpoint once with unchanged INIs; both neutral-link proofs pass.
- XT x64/x86, AT x64/x86 and Model 40 x64/x86 each reach the original
  installer-running checkpoint once with unchanged INIs; their neutral-link
  proofs pass.
- Documentation governance passes. MyNES source/tests/docs/assets have no diff;
  owner INIs have no semantic diff. No MyNES product build was launched.

Ignored S89 helpers/logs are uniquely named `build/s89-*` and retained for the
immediately following board receiver's comparison. Product build handle 99396,
x64 unit/gate handle 62063 and final x86 gate handle 31414 are terminal success.
All eight boot/neutral logs record success; the final Model 40 x86 handle 89743
is terminal success. No owned S89 build or boot job remains active.
The earlier x86 gate rerun 95859 is also terminal success.
The initial x86 unit/gate
handle 49897 is terminal: unit passed, gates failed for the two corrected
inventory issues. Poll live handles rather than restart on observation timeout.

## Reviewed Exit And Remaining Task Boundary

Actual source/build/test/document review, measured code-size, six manifests,
documentation governance and diff checks pass. All eight final stripped 0540
products and unchanged-INI terminals are qualified. The two target deliveries
and their subsequent actual-commit acceptance are indexed below; this evidence
does not substitute for that coordinator review.

This receives only the entire provider-registry batch. Remaining common bus,
AT/XT family wiring and retained D4 ownership are still T540 requirements.

## Qualified Product Identity

All eight products use the fixed S89 reviewed source graph based on S88.
The delivered implementation commits below will identify this same graph;
no source change or rebuild intervenes after the recorded qualification.

Shared P1 `4e23f14b64361e1118b9a8af92dac45ad32c6116` delivers only src/test x86.
NXVM P2 removes the old App files and connects the same qualified source graph;
the intermediate Shared-only commit is not a claimed new NXVM runtime baseline.
Coordinator review inspected both actual commits, their target boundaries,
mechanical source comparison, original assertions, verification logs and eight
artifact hashes. No MyNES or owner INI change is present. Acceptance closes S89
only; the ledger retains common bus, family wiring and unused mode-binding work.

| Product path | Bytes | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1335049 | `F72C5F9594372BCB381DD9D86F32F7D2B8A2723DE517B2E802FCF8621BDEF869` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1505881 | `3926A86B515486D50044FA88945758DC4CBCC5F0EEF3A36984F125A156A53F49` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1351366 | `07101A49359A33B204A578BE1190A9AB39FD1D89C05F7C457434A3FFA0085D54` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1522196 | `5D30C2862BD05D7C79873DE0BE3AA749BB1CDA43EDA84F7E8E964AF209FB2860` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1351334 | `6D436070A61D7C22BA28358D4436F44C16DCF6423DE85246B9F2B17539DFA09B` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1522163 | `C78551E6D73A3F22536F033C69D31FBF6AAC6E80966C48A6C4CEB0BC4E81CF25` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1351400 | `B70CE34BD1D3AD3BD7265D6C559B2DE162E8C3B71AFECE1881C0AFC56DC5644A` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1522231 | `909F912D0C69B98CD1C56D7FAD045CA431F1DBA8776C73435353C3943A5B2225` |
