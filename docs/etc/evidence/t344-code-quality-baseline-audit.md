# T344 Code-Quality Baseline Audit

## Scope And Method

This read-only baseline audit examined the tracked C/C header source tree,
configured GCC/Ninja build graph, current-gate registration, coding-rule
mechanical constraints, existing debt records, and representative high-risk
owners.  It is an evidence record, not a new coding authority.

The audit covered 330 C files, 104 headers, and approximately 123,033 source
lines under `src/` and `tests/`.  It used the configured MinGW GCC 16.1.0
toolchain, actual Ninja compile commands, a separate clean CMake configure,
the existing T317 strict-smoke verifier, documentation governance, and
targeted static searches.  No code was changed during the audit.

## Verified Healthy Boundaries

| Boundary | Result | Evidence |
| --- | --- | --- |
| Current runtime regression | Accepted | T343 reran `current-gate`: 218/218 tests passed. |
| T317 selected smoke strictness | Accepted, bounded | `verify-t317-strict-cpu-smoke-coverage` found all 47 listed direct Ninja source commands carrying `-Wall -Wextra -Wpedantic -Werror`. It is not a whole-tree claim. |
| Project fixed-width type vocabulary | Accepted | The only `uint*_t`/`int*_t` spellings in `src/` and `tests/` are the 14 foundational aliases in `src/type.h`. |
| Header inclusion guard baseline | Accepted | All 100 scanned `src/` headers have an include guard or `#pragma once`; no test directly includes a production `.c` file. |
| Abrupt exits, assertions, and warning suppressions | No mechanical hit | The scan found no direct production `abort`/`exit`, `assert`, compiler-warning suppression, or raw fixed-width type leak outside the type facade. This does not prove every runtime error path. |

## Findings

### F1 - Clean Configuration Is Not Reproducible (P0)

A fresh CMake configure in `build/audit-gcc` fails before generation:

```text
Current-gate #UD source lacks T337 inventory: core-machine-vm86-delivery-smoke
```

`core-machine-vm86-delivery-smoke` is listed in
`PROJECT_CURRENT_SMOKE_TARGETS` and its source contains a `VCPUINS_EXCEPT_UD`
assertion, but it is absent from
`PROJECT_T337_UD_CURRENT_GATE_TARGETS`.  The existing configured build can
remain green because that source-sensitive CMake validation is evaluated at
configure time rather than at every incremental build.  The target must be
given one truthful T337 `#UD` disposition; the evidence indicates the retained
non-real-negative classification rather than a removal or weakening of the
gate.

### F2 - Strict GCC Coverage Is Partial and Not Globally Measured (P1)

Actual `ninja -t commands all` output contained 331 direct C compile commands:
144 carried all four strict flags and 187 did not.  The command set represents
323 distinct sources because some sources are compiled into more than one
target.  In the direct production command sample, 30 of 84 commands were
strict; 54 were not.

The non-strict group includes the mixed inherited `core-machine-executor`
sources and VM composition/platform sources.  This agrees with, but is broader
than, the existing inherited-NXVM strict-compilation debt.  A direct strict
syntax probe of the executor found `memory.c` signedness diagnostics and a
large diagnostic set in `cpu_instructions.c`; these are not a reason to add a
global `-Werror` or to misrepresent linked strictness as direct compilation.

The T317 verifier remains valuable but intentionally limited to its 47-owner
inventory.  A durable whole-graph matrix must distinguish retained strict,
newly strict, and deferred inherited/mixed targets with an exact reason.

### F3 - Current-Gate Source List Contains Duplicates (P2)

The raw `PROJECT_CURRENT_SMOKE_TARGETS` list has 291 entries but only 207
unique target names.  Duplicates are eventually removed only after composition
into `PROJECT_CURRENT_ALL_SMOKE_TARGETS`, so current CTest behavior is not
duplicated.  However, the raw list is also consumed by source-sensitive
configuration checks, where repeated declarations increase drift and review
risk.  The list should become unique at its ownership boundary and receive a
narrow configuration-time uniqueness check.

### F4 - Historical Smoke Fixture Repetition Remains (P2)

Fifty-four `tests/machine` sources repeat the create/bind/freeze/reset shape.
The largest owner smoke is 3,075 lines.  The existing TODO correctly limits
the already-completed T332 fixture work to its fixed 47-source inventory and
requires a later semantic classification before shared-helper migration.  The
new task must not replace distinct provider, GDT/IDT, or device setup with a
generic mirror fixture.  It should create the fixed source-to-shape inventory,
identify genuinely equivalent setup phases, and either make a narrow
test-support-only extraction or record explicit retained variants.

### F5 - Bounded Legacy String Risk Persists (P2, Deferred)

`core_product_debug` and inherited xasm internals still use the unsafe legacy
`STD_STRCPY`/`STD_STRCAT` facade.  T335 closed the public xasm capacity and
failure contract, but did not make all legacy internals capacity-aware.  This
is already separated by the xasm/debbugger debt boundary and must not be
patched opportunistically during build-quality work.

### F6 - Large Cohesive Legacy Owners Need Deliberate Review, Not Size Limits

`cpu_instructions.c` (18,306 lines), `aasm32.c` (10,870), and `dasm32.c`
(6,786) dominate the tree.  File size alone is not a defect: the CPU file has
a single instruction-execution owner and 536 internal helpers.  Any split or
warning cleanup must preserve instruction-state, fault, validation, and commit
ownership and use the existing holistic-execution-path debt process.

### F7 - Shared Debug Completion Drift Broke Two Current-Gate Owners (S1)

The first full S1 current-gate run reproduced two unrelated-looking failures:
`current.core-machine-pushf-popf-s47-smoke` reported its attribute stage, and
`current.core-machine-software-int-s50-smoke` returned without its success
marker.  Rebuilding the exact targets reproduced both failures, so neither was
masked as a stale executable or removed from the gate.

The common cause was the T341 debug-completion owner.  It queued a TF
single-step trap solely from the pre-instruction TF state, even where a
successful interrupt gate had cleared TF at the architectural completion
boundary.  A software `INT` could therefore receive an erroneous later #DB.
`_debug_complete_instruction` now requires both the pre-instruction trace
state and post-completion TF before scheduling BS.  The only use of
`debug_tf_before` is this owner; focused T341 TF/DB, S47, and S50 tests cover
ordinary tracing, RF clearing, and the software-interrupt boundary.

The S47 assertion was also stale: RF is consumed for a successful non-IRET
instruction by the same debug-completion mechanism.  Its PUSHF/POPF attribute
matrix now proves that RF is not pushed, is cleared after successful PUSHF and
POPF/POPFD execution, and that VM/reserved-bit treatment remains unchanged.
No CPU instruction encoding, interrupt-frame serializer, public ABI, or
current-gate membership was weakened.

## T344 Admission Consequences

T344 orders four bounded S units:

1. repair clean-configuration reproducibility by reconciling the T337 `#UD`
   inventory;
2. produce and enforce an exact direct-source strict-compilation coverage
   matrix without global `-Werror`;
3. make current-gate source registration unique and mechanically checked; and
4. classify the remaining historical fixture shapes, then make only a proven
   test-support extraction or record retained variants.

The task explicitly leaves inherited executor warning cleanup, xasm internal
capacity redesign, and non-equivalent fixture/device setup to their existing
or newly recorded admission boundaries.

## S1 Clean-Configure Reconciliation

S1 classified `core-machine-vm86-delivery-smoke` as a T337 current-gate `#UD`
owner with the retained non-real-negative disposition.  Its `#UD` assertion is
inside the VM86-to-protected delivery fixture, not an unowned real-mode
terminal or a new vector-6 policy producer.  The target is now listed once in
the T337 owner inventory and once in that disposition list; its existing
current-gate membership is unchanged.

Fresh configuration in `build/t344-clean-gcc` succeeded with MinGW GCC 16.1.0.
The clean build of `core-machine-vm86-delivery-smoke` and the exact
`current.core-machine-vm86-delivery-smoke` CTest both passed.  This validates
the configuration-time source scan without weakening it or changing guest
execution behavior.

The complete configuration-time sweep continues to inspect every listed
current-gate target source for `VCPUINS_EXCEPT_UD`, `_SetExcept_UD`, or
`UndefinedOpcode` and rejects a missing, conflicting, or duplicate terminal,
real-delivery, or explicit non-real disposition.  S1's added VM86 target is
the sole new listed disposition.

## S1 Current-Gate Recovery

After the clean configuration, exact rebuilt tests for VM86 delivery,
PUSHF/POPF S47, software INT S50, and the T341 TF/DB owner all passed.  The
complete current-gate result is 218/218 passing in 12.16 seconds real time.
The task artifact `build/output/nxvm_0_5_0344.exe` was rebuilt from this
source graph with SHA-256
`84674E5B32F3CD5C21834F23277E46BEC86156958878D4A6DD5223D325BD74A2`.
This recovery keeps the gate as a runtime oracle: it neither excludes the two
tests nor accepts a binary built before the source repair.

## S2 Direct-Compilation Matrix

The T344 CMake inventory combines the 18 declared production targets with the
canonical current-gate target list, removes duplicate target names at that
ownership boundary, and emits one generated row for every direct C source:
`target|source|status|reason`.  This is a direct-command matrix, not a claim
about libraries linked by an executable.

The GCC/Ninja baseline contains 236 in-scope targets and 305 direct C source
rows.  The verifier builds all retained-strict targets, obtains the actual
`ninja -t commands <target>` command for every row, and rejects a missing or
duplicate target/source row, an unknown status, an empty reason, a strict row
without all four flags, or a deferred row whose actual command is already
strict.  The resulting dispositions are:

| Disposition | Direct C rows | Meaning and admission condition |
| --- | ---: | --- |
| Retained strict | 130 | The target directly declares `-Wall -Wextra -Wpedantic -Werror`; actual command verification is mandatory. |
| Deferred production | 54 | The source belongs to a mixed or inherited production target, including `core-machine-executor`, VM composition/platform/product layers, and the retained type facade. A later task must establish that target's source ownership and clean its warnings without changing inherited behavior before adding target-local flags. |
| Deferred owner test | 121 | The current-gate executable's own test source has no proven target-local strict build. Its exact admission condition is a target-local warning baseline and remediation; linking to a strict library is explicitly insufficient. |

No target was newly promoted to strict in S2: the audit did not silently turn
the 175 deferred direct commands into quality claims or scope a broad warning
cleanup under this matrix task.  The generated
`t344-direct-compilation-matrix.txt` remains the reproducible row-level
evidence for the configured build, while the committed verifier is the
mechanical closure gate.

## S2 Fresh-Rebuild Current-Gate Recovery

The strict matrix verifier rebuilt owners that had previously been satisfied
by stale executables.  That exposed three obsolete test contracts:
`core-machine-protected-16-external-s4-smoke`,
`core-machine-protected-16-outer-s5-smoke`, and
`core-machine-hardware-delivery-s3-smoke` set TF while asserting an unrelated
external NMI/PIC delivery result without installing a vector-1 handler.  The
accepted T341 mechanism requires a sampled TF trap before a pending maskable
PIC IRQ; its dedicated `core-machine-tf-db-s60-smoke` is the sole owner of
that ordering.

The three external-delivery owners now initialise only CF|IF for their
hardware-origin rows and expect that same saved FLAGS image.  Software-INT
rows in the S4 owner retain their TF-specific frame coverage.  The repair
does not alter `ExecInt`, debug scheduling, gate serialization, or interrupt
priority; it removes the stale cross-mechanism claim and its committed
diagnostic print.  Exact S4, S5, S3, and T341 focused tests pass after the
repair.  Fresh configuration, the 297-row strict matrix verifier, governance,
diff check, and the full 218/218 current-gate also pass.

## S3 Current-Gate Registration Integrity

The raw ordinary list had two duplicate target identities:
`core-machine-debug-mov-s59-smoke` and
`core-machine-tf-db-s60-smoke`.  Their later aggregate
`REMOVE_DUPLICATES` concealed the defect rather than rejecting it.  The
ordinary list now contains each target once.  The five deliberately
media-classified targets that also appear in the ordinary membership list
remain unchanged: media classification controls their argument-bearing
registration, not a second CTest member.

`project_t344_verify_current_gate_partition` now builds one explicit
canonical union: it rejects a missing or duplicate ordinary target, a missing
or multiply classified media target, and preserves a deliberate ordinary/media
reference as one canonical member.  Registration records every target in a
global configure-time property and rejects a second registration.  The new
`verify-t344-current-gate-registration` parses the generated
`CTestTestfile.cmake` and rejects duplicate, missing, unlabeled, or
out-of-canonical `current.*` tests.

The canonical set has 218 current-gate targets and the generated CTest file
contains exactly those 218 labelled registrations.  The same canonical set is
now the input to the direct-compilation matrix, correcting S2's former
ordinary-list-only input.  Its regenerated result is 305 direct C rows across
236 production/current-gate targets: 130 retained strict, 54 deferred
production, and 121 deferred owner-test rows.  No member, media argument,
label, timeout, or smoke behavior changed.

The S2 scan also found one distinct later cleanup: identical target-local
strict options are declared twice for
`core-platform-presentation-mailbox-smoke`.  It does not weaken compilation
or affect registration, but must be removed by a later bounded T344 corrective
subtask together with a complete strict-declaration duplicate sweep.

Fresh configuration, the registration verifier, the regenerated direct matrix,
all 53 specialized verification targets, documentation governance, diff check,
and the full 218/218 current-gate pass.  The source-level guards are the
negative proof: a duplicate ordinary entry, a duplicate media classification,
or a nonexistent target stops configuration before any CTest registration;
the generated-file verifier independently rejects an incomplete, duplicate,
unlabeled, or out-of-canonical CTest result.
