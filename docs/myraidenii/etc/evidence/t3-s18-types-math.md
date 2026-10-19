# T3 S18: Approved Optional Types Math Vocabulary

## Admission And Ownership

Baseline bf7e69c89 is clean and pushed. The owner explicitly answered
"包含，批准这六文件有限改动" to S12's six-file review request. The coordinator
admits the next linear S18 prerequisite; the executor confirms its packet.
This is additive declaration vocabulary, not COP implementation or a reduced
T3 gate. The complete S12 nine-operation ledger and S13 status-prefix correction
remain the required numerical receiver after this prerequisite.

Exactly six Shared files change: optional src/lib/types/math.h, Types README,
the existing Types contract test, its existing test/lib CMake registration,
and the two Lib manifests. Shared P1 owns those files alone. MyRaidenII P2
owns the task packet, evidence, receiving proposal and acceptance records;
it introduces no additional Shared implementation. All tests and actual-diff
review precede delivery/acceptance. Existing products are named receiving
nonusers, not unauthorized change targets.

## Design And Similar-Issue Review

The12-line optional header includes ISO C math.h and exports exactly four
one-to-one macros: lib_c_sin, lib_c_cos, lib_c_atan and lib_c_sqrt. The standard
library remains the implementation and owns domain/error behavior. There is
no algorithm, mutable state, float/double alias, new implementation library,
OS selector, umbrella inclusion or global link dependency. The existing
declaration/layout gate already admits math.h and derives forbidden raw names
from aliases; no gate weakening or registration-helper edit is necessary.

The existing test adds four volatile double-to-double function pointers and
four exact-value checks: sin(0)=0, cos(0)=1, atan(0)=0 and sqrt(4)=2. This
checks actual calls rather than folding literal expressions, without another
test executable or a numerical approximation test suite. Its existing target
alone links m on UNIX. Windows retains its existing CRT linkage. This UNIX
branch is reviewed structurally, not claimed as an executed Linux build.

Searches of src/lib and test/lib for raw sin/cos/atan/sqrt calls find no old
consumer needing migration. The same tracked C/H search across src and test
finds no duplicate raw-call implementation. Search for lib/types/math.h and
the four new aliases finds only the new header and the existing Lib contract
test as code consumers. Types interface/other consumers remain unchanged;
the future COP consumer supplies its own scoped link when admitted. No FPU
private dependency, parallel math facade or compiler-specific fallback exists.

The actual x64 contract object disassembly contains four indirect calls through
the volatile pointers. Both target executables pass; x86 also imports all four
CRT names directly, while x64 uses the existing toolchain's CRT/math linkage.
The test proves declarations/signatures and exact basic runtime calls, not
physical COP precision or exhaustive platform math behavior.

## Change Size And Verification

Initial estimate Shared +30--40/-3--5 includes declaration documentation and
manifest rows. Actual six-file delta is +34/-3/net31: header+12, README+5,
existing test+10, CMake+3, source manifest+2/-1 and test manifest+2/-2.
Counted C/header/CMake source-and-test delta is +25/-0/net25, with no
production implementation change. Documentation/manifests are separated
from that execution-rule code count. No removed path needs a receiver:
the existing Types declaration owner, test and standard-library implementation
are retained; the added lines are precisely the approved missing capability.

Both stable cached Ninja graphs build build-unit-tests with -j8 under strict
warnings. Each recompiles/links only the existing Types contract test after
CMake regeneration. A subsequent build on each reports no work to do.
Source manifest, Types declaration/layout, documentation governance and
whitespace checks pass. Focused x86 Types contract passes1/1,0.10s.
Focused x64 declaration/boundary/manifest checks pass9/9,105.60s aggregate;
its exhaustive layout self-test takes104.99s. Final complete repository unit
suites pass687/687 per width, zero failures/skips: x64 113.60s and x86 126.35s.
Both runners terminate exit0 under the unchanged per-test budgets and the
600-second aggregate bound. Final LastTest.log SHA-256 identities:

- x64: 9BF1698666F480F2738C01A1AE9BCDBC8FB8D1C548354610CE47C14784DC07A0.
- x86: 45F3B0213F8787B3D0FD1557E862FD99EC72FDD6CE68CA2927379F637D6AED61.

Frozen header SHA-256:
3A4FC7B527C3D8CE018838CB8FD259F383D817DBED3D67094DFD48D12EC2A688.
Frozen existing test SHA-256:
C443CD5800B126DA8F5DC26FFF54AE142035A55EBE1519199AF3CC48FFB43F9A.
Source manifest SHA-256:
F93120DCE8E2D5C22E656EAED4440044C8DC0025B279185C50F6189C2ECD077B.
Test manifest SHA-256:
2BBDDF388A33A35CCD8191DA7727E54DE7150BD7AEDA591FB307FB9FF80C1047.

Ten existing release identities,33 tracked asset guards and six external
media-role hashes match the accepted S10/S49 records. They are rechecked at
delivery. No product uses the optional header, production/link inputs are
unchanged and no unaffected EXE rebuild/refresh is warranted. No ROM is used
by these tests. Build contains only .ccache and .ninja; no task output root,
external source payload, cleanup deletion or owned background residue is
created. No desktop/RDP, native-audio, external integration or playable-game
qualification is claimed. T3 and M1 remain open.

## Coordinator Acceptance

After switching roles, the coordinator reads the complete actual git show of
Shared P1 9b41a82ca, not only its summary. Exactly the six approved files
change, +34/-3/net31. All four macros are additive declarations; the test uses
real runtime calls, and the UNIX library is private to its existing target.
Both canonical manifests include exactly the changed/new hashes and pass the
full suite. No current product uses the header, receives a link change or
needs a binary refresh. There is no algorithm/state/API expansion beyond the
four explicitly approved names. Packet, source/layout rules, original request,
full qualification and actual change agree; the coordinator accepts P1.

The product-only P2 records this acceptance, removes the active packet and
keeps all unfinished numeric/board/startup gates open. Its records do not
alter frozen code/test/log identities. S18 closes its prerequisite only;
the complete COP numerical receiver and playable MyRaidenII remain open.

## Archived S18 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S18 after accepted S17 bf7e69c89; sequential coordinator/executor in one session. |
| Admission And Approval | Owner explicitly answered "包含，批准这六文件有限改动" to the S12 six-file Shared math vocabulary review. Same-T ordering is delegated. Shared P1 contains exactly the six approved files; MyRaidenII P2 records the complete delivery and coordinator acceptance. |
| Objective | Supply the approved optional ISO C sin/cos/atan/sqrt vocabulary prerequisite for the complete B07/S12 numeric receiver. |
| Non-goals | No COP algorithm, floating scalar aliases, mathematical implementation, umbrella inclusion, global math linkage, other Shared API, RNG/DMA repair, gameplay or release refresh. |
| Reference Baseline | Clean pushed bf7e69c89; S12 math proposal and complete numeric ledger, S13 entry-status correction and S17 dependency partition. |
| Candidate Proposal | ../proposals/m1-board-cop.md and ../etc/evidence/t3-s12-cop-geometry-contract.md; this prerequisite does not qualify any unfinished numeric operation. |
| Files And ABI Surface | Exactly src/lib/types/math.h, src/lib/types/README.md, test/lib/types/types_contract_smoke.c, test/lib/CMakeLists.txt and both Lib manifests. Four additive lib_c_* macro names, no changed signature/type/enum/layout. Estimate Shared +30--40/-3--5; product records separately. All existing consumers are optional-header nonusers; the existing Lib contract test is the only new consumer. |
| Applicable Rules | Architecture: Types declarations only, no new owner/state or dependency inversion. Coding: canonical aliases, unchanged CRT behavior and existing test reuse. Execution: one target per P, strict cached builds, full proof, actual-change review and clean delivery. Documentation: preserve historical S12 approval request and record this subsequent authorization. Source: original declarations only, no third-party implementation or media. |
| Verification | Stable x64/x86 Ninja build-unit-tests -j8, focused lib.types/types_contract and Lib gates, then complete repository unit suites sequentially with four workers and 600-second aggregate budget. Check manifests, Types layout, dependency/negative tests, documentation, whitespace and exact six-file Shared scope. Confirm generated runtime calls through volatile pointers and preserve ten release/33 tracked-asset/six media-role hashes. |
| Expected Markers | Optional math.h exports exactly lib_c_sin/lib_c_cos/lib_c_atan/lib_c_sqrt; existing target links m only on UNIX. No layout gate weakening, Types umbrella change, new test executable or global linkage. |
| Asset Needs | ROM-free tests; no external input or temporary task directory. Preserve release identities because no existing product includes this opt-in header. Reuse build/.ninja and build/.ccache only. |
| Reporting Requirements | Report approved boundary and estimate before code, actual source/test line delta, dual-width results, unchanged artifacts, similar-issue sweep and remaining numerical receiver. Record complete proof before Shared P1 push; actual coordinator review precedes product-only acceptance P2. |
| Stop Conditions | Any seventh Shared file, changed existing API/behavior, platform selector, new mathematical implementation, raw CRT consumer bypass, unrelated product/input change or unapproved source derivation. |
| Exit Criteria | Four real runtime aliases and existing Lib gates pass on both widths; full units pass, manifests match, complete scoped P commits are pushed and actual-change review accepts the delivery. T3 numeric/game obligations remain open. |
| Original Owner Request | Complete playable MyRaidenII under single-session dual-role governance; explicitly approve this six-file S12 mathematical vocabulary addition. |
| Similar-Issue Sweep | Search existing Lib production/test raw sin/cos/atan/sqrt calls and optional header ownership; retain one declaration owner, existing tests and scoped consumer linkage. Verify existing products do not include the new header or gain a math link. |
