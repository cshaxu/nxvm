# T540 S87 Construction And Private-State Boundary

## Intake And Executor Confirmation

Baseline is accepted S86 P2 `8f0557815`, following complete P1 `5654912d7`.
The executor confirms CURRENT's complete constructor/header/direct-fixture
boundary under the owner's automatic linear-S authorization. This is NXVM
only. No Shared, MyNES, owner INI, external master or physical source movement
is admitted here; the final physical extraction remains required by T540.

The current neutral Core layout retains one named board pointer. Production
reads/writes now occur only in the board factory and its construction helper;
all electrical, display/input/controller operations already use actual board
handles. The driver has a separate legitimate borrowed board handle: identical
field spelling does not imply identical ownership.

Board creation allocates one attachment, binds its copied callback set before
fallible setup, initializes clocks/chips/routes in existing order and delegates
all post-binding failures to Core destruction. Before binding succeeds, the
board allocation has its own explicit cleanup. Preserve this exact ownership
transfer. Return the actual board from the existing factory only on success;
do not recover it through a getter or attachment-context cast.

Neutral constructor input/validation and allocation-failure seams are still
declared in private machine.h. Board state includes that layout transitively.
This requires a real production constructor boundary and a separately owned
test-failure solution, not merely changing an include name. The two existing
failure fixtures cover five seam calls, including every constructor port
allocation failure across default, auxiliary-PIT and XT variants. Those cases
cannot be silently deleted or weakened to remove a dependency.

The S86 intake regex identifies 91 direct-board candidate test files. Each
must be inspected for actual Core, fixture or driver ownership before final
classification. Constructors already publish optional borrowed board outputs;
tests should retain those actual handles where they own board observations.
Core-only private assertions remain Core-owned; profile/firmware scenarios
remain product-owned. No classification or complete migration is claimed yet.

## Work Boundary And Acceptance

First replace constructor reads with its local board allocation and direct
success-only output, preserving callbacks and failure order. While direct
fixtures still use the historical Core field, any temporary success association
is an explicitly unaccepted worktree bridge. It must disappear together with
the field before this S can deliver. It is not another public/production path.

Then complete the neutral constructor/header and entire fixture cut as one
receiver. Existing static gates must prove the forbidden private association
and transitive layout cannot return; tests retain original failure and reset
assertions. CURRENT owns full dual-width units/gates, eight fresh products,
neutral linkage and one boot per unchanged INI. No P or S closure is allowed
on a partial constructor edit. Actual Core and flat IBM-PC source/test/build
movement follows this boundary and remains the thread goal.

## Complete Implementation And Verification

Core's named board member and board-state forward declaration are removed.
The immutable thirteen-field executor configuration now belongs to the neutral
production interface. Board construction projects that value once, validates
before allocation, creates neutral Core, then attaches its one local board
allocation. Core and the borrowed board are published only on complete success.
Plan construction uses this same factory. The obsolete internal mixed factory
and its two production allocation-test wrappers are removed.

The private neutral allocation seam still executes the sole constructor body.
The test-owned `board_construction_fixture.h` combines production projection,
that private seam and the real board attachment; it does not recreate device
setup or expose a public test API. The RAM, port-assembly and plan fixtures keep
their allocation failure cases, attempt counts, statuses and null-output checks.
The plan fixture contributes three calls beyond the five originally identified
at intake. Failures after attachment binding still destroy through Core; the
pre-binding board failure retains its explicit cleanup. Helper setup failures
clear newly borrowed board outputs after destroying their Core owner.

The baseline literal direct-board search has 91 executing candidate files and
one intentional negative-injection script. Their receiving classifications are:

- 63 board or mixed CPU/board fixtures: retain the actual board returned by
  construction, in a local variable, owning test state or helper parameter.
  CPU-private assertions stay with their existing owner. Mixed fixtures are
  not declared neutral Core-only tests merely because they execute instructions.
- One neutral-link fixture: the old absent-board check becomes an absent copied
  attachment check; neutral construction now also rejects invalid input and
  clears an existing output before failure.
- 19 product driver/composition fixtures: use the driver's already existing
  borrowed board; raw-Core HDC/RTC helper parameters receive the actual board.
- Seven external integration fixtures and one profile firmware fixture: retain
  their product ownership and inputs, using the driver/capture board handle.
- The negative-injection script remains an intentional prohibited lookup, not
  a runtime consumer. The similar-issue sweep also finds the differently named
  Core pointers in the two-instance isolation fixture; its board and CPU/RAM/
  port isolation assertions remain.

No test file, assertion class or runtime scenario has been retired. The
transitive include closure of `machine_board_state.h` contains no private
`machine.h`, `memory.h` or `port.h`; its neutral arithmetic clock dependency
is not a Core layout dependency. The controller-authority gate checks this
closure and rejects test seams or board layout in the neutral public contract.

Complete delivery verification, before coordinator acceptance:

- All registered executable unit targets build under x64 and x86.
- Final x64 complete unit refresh after comments/formatting: 470/470 passed,
  266.94 seconds; the earlier complete run passed in 163.90 seconds.
- Refreshed x86 complete unit run after final comments/formatting: 470/470
  passed, 79.35 seconds; its complete specialized gates also passed.
- Both-width complete specialized gate targets: passed, including the strict
  direct-compilation matrix and controller/construction boundary.
- Constructor cleanup and neutral-link focused checks: 2/2 passed.
- Five new copied-source construction negatives plus restored positives pass;
  the existing 34 electrical/publication negative probes also pass unchanged.
- Default x64 and x86 product builds and their independent neutral-link checks
  passed. Each unchanged-INI boot ran once and reached the DOS prompt. XT x64
  and x86 also completed their builds and neutral checks; their one unchanged-INI
  boot each reached `installer-running`. AT x64 also completed its build,
  neutral check and one boot to `installer-running`. AT x86 also completed its
  neutral check and one boot to `installer-running`, after the native unit suite
  finished. Model40 x64 completed its build, neutral check and one boot to
  `installer-running`. Model40 x86 completed its build, neutral check and one
  boot to `installer-running` after the final x64 verification process exited.
  Thus all eight rows passed once; no successful row was repeated.

Final header review corrected the neutral constructor comment from stopped to
initialized, matching its actual `CORE_MACHINE_INITIALIZED` lifecycle. The
construction-eligibility comment now annotates its intended query rather than
the executor configuration. These are comment-only corrections, not lifecycle
or ABI changes; final artifact freshness review must account for this edit.

Constructor/destructor review confirms the neutral constructor retains its
original CPU/FPU/clock/RAM/port initialization order and sole body. Its added
preflight rejects invalid inputs and clears the requested output before any
allocation. Before attachment binding, board cleanup releases the local board
and then Core; after successful binding, Core destruction invokes the copied
attachment finalizer once. The plan still delegates topology failure to that
same destruction path. This review does not replace the remaining products,
fixture review or actual committed-diff acceptance.

Allocation-failure and publication fixture diff review retains RAM attempts,
all port failure positions, plan success/failure output identities and IRQ
assertions. Driver/ROM/integration observations now use the driver's existing
borrowed board handle; RTC/HDC helpers receive that same handle explicitly.
The two-instance test still compares independent RTC/FDC/HDC identities as well
as Core CPU/RAM/port identity. Review also aligned five newly touched argument
or declaration indentation sites; no assertion or execution order changed.
Both full units and specialized targets passed after these comment/format
corrections. Final x64 direct compilation covered 402 rows, 377 retained
strict and 25 explicitly deferred. Model40 x86 boot ran only after that
verification process returned success. Final construction/electrical negative
probes, documentation governance, selected local links and diff checks pass.

The first x86 refresh build was explicitly cancelled, not counted as a failure
of a runtime test: the owned Ninja remained live with unchanged process counters,
pending compile work and no compiler children; an outside-sandbox dry run read
the same graph immediately. Its CMake parent/tree and absence of workers were
verified before stopping only that Ninja. After the parent returned terminal
failure, the unchanged script resumed outside the sandbox and immediately
compiled its pending work. No unrelated process or file was removed. This
supports an execution-environment distinction, not a claimed compiler/source
defect or a reason to weaken verification.

The construction gate's literal multiline include regex was replaced by an
equivalent explicit single-line quoted-include pattern. Its positive check and
five construction negative probes pass after the readability correction.

An earlier x64 full run was intentionally cancelled at 107 completed cases
after review found its partial-construction fixture still supplied a null
required internal output. That fixture was corrected and rebuilt before the
complete passing run above. The cancelled run is not closure evidence. Initial
build failures exposed unmigrated HDC and two-instance fixture accesses; those
were migrated with their original checks preserved, not bypassed.

Final counted source/test/gate delta is 104 tracked paths, 1,234 added and
1,123 removed lines, net +111, plus the new 35-line test fixture (net +146).
Method: `git diff --numstat` over NXVM source/test and the changed gate;
documentation, ignored harnesses and artifacts are excluded. Added lines own
actual borrowed test handles, the neutral constructor rejection cases and the
transitive boundary check; there is no second production board or constructor.
Actual diff review distinguishes exact driver handle substitutions from the
Model40 capture, RTC/HDC helper, two-instance and allocation-seam changes.
The latter retain their original operations, expectations and order while
passing actual board handles explicitly. The added neutral rejection matrix
tests the genuine production preflight, not a test-only constructor contract.

## Refreshed Artifact Identity

All eight 0540 products pass PE architecture, revision banner, absence of
compiler-debug sections and freshness against the changed construction inputs.
Default products were refreshed after the comment-only header correction using
the already-generated Makefile2 `/all` targets: these retain recursive dependency
and compilation checks, unlike `/fast`; no configuration/source-list input
changed after generation. Boot completion remains a separate requirement.

| Product | Bytes | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1335049 | `07740E4A95B03F227544DB98D29269A970749ADD8EB9B1F61E7CBF7EA85D596C` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1505881 | `3C3420B5C661F9ED7DC8648E393A82E6DF1142B7561F15BE267694C73C94E74E` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1351366 | `800CA49769ABD785BE7D1C2DF91007D0813EC50E83C18679392754D293355BBC` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1522196 | `FF8D495B65E683A33C30144B9963728B8CF6E7FFC58C1B8CBC79FE3656DBFBC6` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1351334 | `3F424D21DF2914CC6E0ABB8E0819CC0242A77537BC5DE460FC9A0564FAC1B921` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1522163 | `EE6A1E6079B6E0AA5C46BB766AFF314BBE718DFF8EAEF3D5D07C83373270B4AC` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1351400 | `9FB47845E18E2CABBAA0F060F9706380EDB83E49A7F57127A42C54C8CC005C80` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1522231 | `6A31481C0A373BB03E9440E4B276AF3D9B7C960481241D2DE58935EB3621C905` |

Coordinator accepts immediately pushed P1 `c662ecd080acc77be2656005c778fbd3886f07f2`
after actual-commit review: all 119 paths match the reviewed working blobs and
origin/master; they comprise six production, 98 test, one gate, six document
and eight artifact paths. No out-of-target or owner INI path is present.
Governance P2 closes S87 only. T540 remains open.
Actual `x86/core` and flat `ibmpc-*` source/test/build relocation is still
required; this complete boundary cannot substitute for it. The next receiver
must deliver the whole neutral Core component, not more per-function preparation.
