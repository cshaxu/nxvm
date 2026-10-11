# T553 S12 Core And Four-App Quality Audit

## Scope Freeze

This audit owns only the current NXVM private Core and four fixed App roots:

- `src/core` and `test/core`;
- `src/app-my5160` and `test/app-my5160`;
- `src/app-my5170` and `test/app-my5170`;
- `src/app-mydeskpro386` and `test/app-mydeskpro386`; and
- `src/app-nxvm` and `test/app-nxvm`.

The associated CMake registration under `cmake/nxvm/` is in scope only where it
constructs, links or registers one of those owners. `src/lib`, `src/emulator`,
`src/x86`, their test packages, firmware/media inputs, INIs, snapshots and
deployed executables are excluded from this audit.

## Owner Batches

| Batch | Current owner groups | Audit questions | Initial disposition |
| --- | --- | --- | --- |
| A | `core/chips`, `core/x86` | chip/executor ownership, interface exposure, state/time publication, direct owner tests | Verified clean; one duplicated *test* fixture is recorded under F, not a production ownership defect. |
| B | `core/board-base`, `core/board-at`, `core/board-xt` | board glue, routing, deadlines, reset/teardown, profile-free construction | Verified clean; route removal occurs only in infallible `void` teardown and all live construction goes through the frozen board-plan path. |
| C | `core/machine`, `core/product` | driver lifecycle, media/configuration boundary, product-family reuse, failure cleanup | Verified clean in code; one direct architecture-document contradiction is recorded for repair. |
| D | four App `product` roots | fixed identity/binding, entry/teardown, App-to-Core direction and duplicate behavior | Verified clean; the small repeated binding shape is intentionally per executable and has no duplicated lifecycle or composition owner. |
| E | four App `profiles`/`firmware` roots and matching tests | immutable machine facts, external asset boundary, configuration validation and test ownership | Verified clean; profile distinctions are actual XT, IBM AT, DeskPro/D4 and default-AT hardware facts. |
| F | `test/core`, `test/app-*`, `cmake/nxvm` | test/component ownership, independent registration, fixture scope and build graph | Two exact duplicate Core fixtures are a confirmed test-maintenance defect; no cross-App production or test dependency was found. |

## Required Evidence Per Row

Each row records the exact source/CMake/test query, direct caller or consumer,
one of `verified-clean`, `confirmed-defect`, `not-applicable`, or `transferred`,
and the reason. A confirmed defect additionally records the owning component,
the whole-class search, the intended bounded repair S, and the regression that
will prove the repaired contract.

## Baseline Facts

- T553 S10 already establishes default x64/x86 CTest 567/567 and the three
  profile-specific integration selections on both widths. These are regression
  evidence, not a substitute for this source audit.
- Core's current build graph declares fourteen chip targets, three board targets,
  one execution Core, and one machine driver. The four Apps are fixed bindings,
  not peer dependencies.
- Historical controller and board ledgers are context only. Every current-source
  disposition in this audit must be reproduced from the current tree.

## Current-Source Ledger

| Owner / class | Query and direct evidence | Disposition | Result / receiver |
| --- | --- | --- | --- |
| Core corpus boundary | `cmake -DCORE_ROOT=... -P src/core/verify_corpus.cmake`; recursive include scan | verified-clean | The gate passed. `src/core` has no `app-*` include or product-name reference; the declared Core allowlist is only `core`, `lib`, `emulator`, and `x86`. Only `core/product` may consume `x86/product`, and lower Core strata cannot consume `core/machine`. |
| Core manifest | `cmake -DCORE_ROOT=... -P src/core/verify_manifest.cmake` | verified-clean | Passed against the audited current tree. |
| Chips / execution error paths | Current-source review of KBC Pause enqueue, FPU FLD, FDC timing write, CPU/board fault reporters, and their direct chip/board tests | verified-clean | The apparent ignored KBC enqueue calls are preceded by a whole-sequence FIFO-capacity precondition; no intervening operation can consume capacity. FPU FLD stack overflow is a completed guest instruction that records its x87 exception, not a host API failure. FDC programming accepts a fully constructed local timing value. Fault reporting and `void` teardown have no recoverable caller path. No blanket `(void)` rewrite is justified. |
| Board construction and teardown | Current-source review of board attachment, route installation/removal, D4 profile binding, deadline/clock wiring, and board-owner units | verified-clean | App profile construction publishes only after the Core plan freezes. Board destroy operations revoke borrowed routes before release; their `void` APIs intentionally cannot report a retryable result. Model40 remains the sole D4 owner. |
| Machine / product reuse | Review `core/machine/*`, `core/product/{factory,ini,startup}.c`, callers `vm_app_run()` and all four bindings | verified-clean | Machine owns the one Core-to-Emulator driver; Product owns the common NXVM.ini parse/compose/banner route. The four App bindings carry only fixed identity, firmware and profile-prepare facts. No peer-App edge, second driver, or App-side lifecycle/parser was found. `vm_app_ini_load()` deliberately maps unreadable/invalid INI input to its documented family-level internal-startup failure; its owner-local smoke asserts that contract. |
| Four App product roots | `rg app-(my5160|my5170|mydeskpro386|nxvm)` per root; inspect four `main.c`, four `machine_binding.c`, and binding interfaces | verified-clean | Every App references only itself plus public Core/Shared headers. Exact repeated binding shape is required because exactly one executable links a selected global binding; extracting it would require a registry or runtime machine selection that the architecture prohibits. |
| Four App profiles / firmware | Inspect construction functions and profile descriptors; exact content-hash duplicate scan of all in-scope production C/H/CMake | verified-clean | No exact duplicated production C/H/CMake file exists. The 5160, 5170, default AT and Model40 paths have genuine hardware/firmware differences. Project-authored default firmware stays App-owned; commercial assets remain build inputs, not tracked source. |
| Test ownership / registration | `cmake -DTEST_ROOT=... -P test/verify_test_boundaries.cmake`; inspect `test/core/CMakeLists.txt`, app integration registrations and `cmake/nxvm/NxvmProduct.cmake` | verified-clean with named exceptions below | The Core test boundary gate passed. The root NXVM CMake graph is the single selected-App build composition owner; App tests remain physically under their App roots and are labelled from source path. `unit.fdc-boundary-negative` is intentionally App-owned despite its Core test location; the retained evidence names that specific mixed machine-media contract, so it is not renamed in this audit. |
| Core test fixture duplication | SHA-256 grouping of every in-scope C/H/CMake test file; direct callers from both `test/core/x86` and `test/core/board-base/composition` | confirmed-defect | `exception_fixture.h` and `memory_alias_fixture.h` are byte-identical pairs in `test/core/x86/` and `test/core/board-base/composition/`. They are common Core test fixtures: their implementation consumes only the inward `core/x86` public contract, while both Core-x86 and Board tests use them. Repair receiver: a bounded Core-test S moves one copy of each to `test/core/support/`, updates both caller classes and the Core manifest, then deletes the two duplicates. This creates no production edge, no Core-to-App dependency and no second fixture variant. |
| Architecture wording | Compare `docs/nxvm/design/ARCHITECTURE.md` Core Product owner paragraph with its later “Each App owns its INI syntax” claim and `core/product/startup.c` caller | confirmed-defect | The source has one shared NXVM.ini grammar in `core/product`; the later App-owned-INI sentence contradicts both the earlier owner statement and current code. Repair receiver: a documentation-only Core S removes or rewrites that later sentence without changing runtime behavior. |

## Similar-Issue Sweep

- The exact-duplicate scan over all Core/App production C/H/CMake files found
  **zero** duplicate production files. The same scan over all in-scope test
  C/H/CMake files found only the two fixture pairs named above.
- Recursive App-root reference scans found no sibling-App dependency in either
  production or matching test roots. Core and `test/core` have no App-root
  include at all.
- The whole-repository fixture sweep found no other exact duplicate C/H/CMake
  test file. Its cross-test-root include scan finds only outward App-to-Core
  harness use (`test/core/setup`, Core board fixtures and Core machine support)
  plus the two fixture copies above; it finds no Lib/Emulator/x86 test depending
  on Core or an App, and no Core/App test dependency pointing outward.
- A second source-include scan confirms the same declared direction: `test/lib`
  has no Emulator/x86/Core/App include, `test/emulator` has no x86/Core/App
  include, `test/x86` has no Core/App include, and `test/core` has no App
  include. The App test roots include only their own App sources or lower
  Core/Shared contracts; no sibling-App include was found.
- The failure-path sweep searched every current `(void)` call that invokes a
  status-returning operation in App/Core source, then reviewed the distinct
  mechanisms (KBC/FDC/PIT preconditioned delivery, guest-fault reporting, and
  non-retryable `void` destruction). No additional silent-status-loss defect
  was reproduced from those mechanisms.

## Required Repair Admission

This read-only S does not alter code. Before T553 can honestly close, admit a
bounded repair S covering only:

1. consolidation of the two exact duplicate Core test fixtures and its
   manifest/caller updates; and
2. the directly stale Core Product/INI ownership sentence in the NXVM
   architecture document.

The repair must run the affected Core x64/x86 unit selection and the existing
Core manifest/corpus/test-boundary gates. It does not require executable
rebuilds because neither production source nor build input changes.

## Audit Verification Performed

- `cmake -DCORE_ROOT=O:/repos.hobby/nxvm/src/core -P
  src/core/verify_manifest.cmake` — passed.
- `cmake -DCORE_ROOT=O:/repos.hobby/nxvm/src/core -P
  src/core/verify_corpus.cmake` — passed.
- `cmake -DTEST_ROOT=O:/repos.hobby/nxvm/test/core -DTEST_LAYER=core -P
  test/verify_test_boundaries.cmake` — passed.
- `powershell -NoProfile -ExecutionPolicy Bypass -File
  tools/shared/Verify-DocumentationGovernance.ps1 -RepositoryRoot . -Product
  nxvm` — passed.
- `git diff --check` — passed for this audit-record change.

No dynamic unit, setup, integration, desktop, firmware or external-media test
was rerun in S12: this S changes only the audit record and has not repaired a
runtime contract. The T553 S10 dual-width qualification remains baseline
regression evidence, not a substitute for the two admitted repair receivers.
