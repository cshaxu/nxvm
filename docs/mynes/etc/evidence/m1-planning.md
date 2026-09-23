# M1 Admission And Planning Evidence

## Ownership And Authority

Owner requested M1 admission and planning on 2026-09-19 after M0 closure c42a257.
This indexed record belongs to M1 Td S1 and records research, requirement mapping
and review; it is not another queue, active packet or architecture authority.
Current owns actual status, Queue owns order, linked proposals own candidates.
Retain this evidence with Git; implementation histories later carry actual proof.
One session executes and reviews sequentially; no independent reviewer is claimed.
The owner's approval of planning includes its supporting evidence. No source,
test, build scaffold, ROM or sibling modification is part of this delivery.

## Frozen Source Inventory

SoftPC revision: 58ac3943170e2e68879d83b49d184a8a84f6c471. Working tree was clean
at inspection. Counts are all tracked files, including readmes/build/manifests.
Git tree identities fix complete root content; source MANIFEST.sha256 files give
per-unit SHA-256 entries, verified read-only. These are input identities, not a
claim that the future modified neutral corpus is identical to the input.

| Source root | Destination root | Files | Git tree identity |
| --- | --- | --- | --- |
| src/lib | src/lib | 89 | 56a9401b47481747dc21e608354ca198279605d5 |
| src/common | src/common | 32 | 1b362e31ed0d2b14212024372ae33d0463e604e8 |
| test/lib | src/test/lib | 44 | 7700d1f1eea0fb70231028b3501c96138ae24d35 |
| test/common | src/test/common | 22 | 2924e111faddb9244fabb6db576dbfdfabe90b8f |

Total: 187 files. SHA-256 of each source MANIFEST.sha256:

- src/lib: ED27591D2A845FD273C1A2EA8E6A18C2246BEC7F64662B71A60CE090EB7C15BA
- src/common: AC7AD8AD72D0BCB5968E831DF1E508AB6F17AAEE12917603AD2252F6AFD36C04
- test/lib: A11295B057E573C1B44D1F666A92217C30B1D0995BFCCED98BF194CF605CD8F5
- test/common: F658664D2C721684FAFDF77FF18D173C8DBB220C918D9B5A0ABF27D108BBA9FF

NXVM consumer reference remains e5e32089bf7607c4ed5039ff43250c1f16d0232c.
It is a later adapter-review reference, not another source import or a validated
receiver. Whole-repository source pins may advance only through a recorded review.

## License Disposition

SoftPC has no root LICENSE/NOTICE at the inspected pin. The owner expressly replied
on 2026-09-19: authorize MIT reuse of their own code. The grant covers copying and
modifying the selected owned Lib/Common/test material into MyNes. Preserve copyright
and grant evidence in the eventual import provenance/notices. This does not license
independent third-party material or recovered-machine code on its authors' behalf.

A read-only notice search over the four roots found Neko 2012-2014 copyright in
common/xasm32/aasm32.h and dasm32.h; a keyword search alone is not a complete rights
audit. The import task must map all 187 units, examine provenance and transitive
inputs, preserve notices, and stop copying any unresolved unit. Source ownership
authorization is now recorded; source classification is still a concrete import
gate, not a blocker to completing this planning S.

## Observed Gaps And Receivers

| Observed source or behavior | Planning disposition |
| --- | --- |
| Common and test standalone CMake select C17; Lib selects C11 | Import/build proves C11 on x86/x64 with extensions off and explicit warnings. No C11 runtime claim today. |
| Test roots derive ../../src paths | Import/build parameterizes roots and preserves one source/test corpus. |
| Common builds common-debug/common-xasm32 unconditionally; debug ABI exposes segment/CPU records | Neutral contracts remove machine semantics, migrate neutral tests and preserve an external-adapter removal map. |
| Driver has media/state-stream/heartbeat hooks and existing paused rendezvous | Neutral contracts retain useful optional transport with explicit unsupported results, not a second execution route. |
| Session presentation plan uses display == Window OR graphics | Input/presentation injects policy/capability; both route branches get proof. |
| Existing graphical Console is not an ASCII renderer | M3 receiver; M1 reports unsupported honestly and tests routing with fake capabilities. |
| Previous inspection found reset/focus/paused-release, growable queue and lifecycle gaps | Lifecycle and input packages revalidate the full transition/reset universe, not only original sites. |
| Source has Linux leaves | Preserve reviewed source; Windows qualification makes no new Linux support claim. |
| Existing SoftPC and NXVM adapters consume old contracts | Qualification produces affected-consumer migration records; actual sibling integration remains separately admitted. |

Research commands: git rev-parse/status/ls-tree, Get-FileHash, scoped rg for
license/CPU/driver/routing patterns and direct reads of Common interfaces and
Lib/Common/test CMake files. No guest media read or external source acquired.

## M1 Exit-To-Proof Mapping

This finite planning ledger maps all approved M1 exits and research gaps. It
records the receiver and required proof, not implementation completion.

| Requirement | Candidate receiver | Required completion evidence |
| --- | --- | --- |
| Reviewed four-root source/test import | Import/build | Per-unit rights/source/destination hashes, notice retention, 187-input disposition and complete new manifests. |
| Independent builds and C11 | Import/build; qualification repeats exports | x64/x86 builds and full suites; standalone entry points, explicit roots, strict warnings, no sibling dependencies. |
| Neutral reusable debug/driver | Neutral contracts | No concrete CPU layout/command engine; two distinct fake codecs, bounded copy, lease/generation/cancel/unsupported tests. |
| One executor and control ownership | Lifecycle | Frozen state/operation matrix and queue limits; accepted/rejected transition and stale-completion evidence. |
| Construction/replacement/teardown failure safety | Lifecycle; handoff owner | Failure injection, prior-state preservation, reset faults, join-before-free and failed-join resource retention. |
| Input reset, including paused mapping | Input/presentation | Per-source reset matrix, stale make rejection, no held-key resurrection before resume. |
| Equal Window/Console readiness | Input/presentation | Injected routing matrix, real unsupported rejection, both native lifetimes and raw/cooked single-reader handoff. |
| Live configuration mechanism and cooked notifications | Input/presentation | Validate/publish/rollback, cancel/read-once/rearm behavior; no product grammar in shared code. |
| Same source/tests for future receivers | Qualification | Identical hashes in both layouts, four build/test runs, fake adapters and actual-consumer migration table. |
| No premature gameplay/pacing/persistence claim | All candidate non-goals | M2/M3/M4/M5 retain their existing roadmap boundaries; no future task queue. |
| Complete M1 closure | Qualification then coordinator | Every eligible ledger member passes; no shared defect is transferred merely to declare success. |

Sequence: import/build -> neutral contracts -> lifecycle -> input/presentation
-> transfer qualification. This explains dependencies; Queue alone orders the
live candidates. No numeric T or S schedule is preallocated. Exact ABI signatures,
queue sizes and safe-boundary budgets are assigned to their owning implementation
task's pre-code contract with explicit capacity/overflow/cancellation proof.
Planning is complete without pretending those unimplemented contracts are tested.

## Verification And Limits

Executed source checks passed at the pin: Lib/Common source manifests, both test
manifests, and Common dependency/platform verifier. Invocations use cmake -P with
the corresponding LIBRARY_ROOT or COMMON_ROOT. Source status remains clean.
Observed tool identities: CMake 4.3.3; MinGW x64 GCC 16.1.0; i686 GCC 16.2.0.
No compilation or runtime tests were executed for this documentation-only task.

Active and final documentation gates, actual-change review, whitespace and staged
scope checks validate this delivery. The unchanged governance checker does not
need new tests for candidate prose. Its self-tests passed in M0; that is historical
evidence, not a newly executed test claim. Source absence, no remote and untouched
sibling status are checked before local delivery.

Coordinator review checks the original request against every row, dependency
acyclicity, package stop conditions, equal-backend priority, mandatory license
gate, explicit shared-versus-product proof and absence of future task admission.
This closes M1 planning only. M1 itself remains open. The first import/build
candidate still requires implementation-task admission after owner plan review.
Complete local subject: M1 Td S1 P1: admit M1 and plan shared foundation tasks.

Final review result: all planning-ledger rows have explicit receivers and proof;
active/closed documentation gates and whitespace checks passed. No source/build
was created and both inspected sibling worktrees remain clean. No planning
blocker remains; implementation and runtime qualification are still prospective.
