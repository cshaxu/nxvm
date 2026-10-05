# T545 S1 Eight-Corpus Importability Audit

## Fixed Source And Decision

Accept unchanged import of SoftPC's latest committed eight corpora:
`03c979c7ba58e2edafd9ba363400da442161b838`. Later committed HEAD
`9291c8cc18b38926176bd66d4749594f424e79b0` changes none of these roots.
The local immutable archive SHA-256 is
`5A9BC73DB8E541B8D2CAEED9F17EF8C2EC722E54FE1838C8AB20E9A068E516F7`.
Receiver baseline is NXVM `826eccc938da1715d2a4c86c2f6be93118f7fbc8`.

During audit, SoftPC began further uncommitted IBMPC executor-state, first-drive
alias, fallible-cleanup and fixture changes. They are excluded from this fixed
acceptance, not silently combined. A future fixed revision needs re-audit.
Sibling files are read-only. Preserve notices under the owner's recorded MIT
authorization for this owner-authored/inherited code. SoftPC has no tracked
root LICENSE; this report does not invent one or a third-party license grant.
No new firmware/media, Microsoft binary or sibling App is imported.

The [complete inventory](t545-s1-file-receivers.md) records every difference,
source/destination receiver and unchanged-import boundary.

## Production Review

- Lib Audio retains one endpoint/worker owner. Cancellation/join failures
  preserve the object for retry; idle wait handles cancellation or publishes
  failure instead of retrying forever. The sole failure owner wakes space
  and control completion, including attach failure. No public API is added.
- Lib Types adds neutral file facade aliases for tmpfile, rewind, fseek and
  ftell. Storage mode/locking contracts are unchanged. KVM Console README
  clarifies logical mouse coordinates; its implementation is unchanged.
- Common Machine invalidates both frame valid bits, index and generation,
  rather than zeroing large pixel payloads. Capture remains locked and requires
  valid/current generation. First identical text after reset republishes.
  Only this owner accesses private frame buffers; invalid bytes cannot be
  captured as a valid frame.
- x86 removes empty internal bus initialize/finalize wrappers and their sole
  constructor/destructor calls. Real memory/port lifecycle remains. CPU,
  assembler, decoder and timing implementation bytes are unchanged.
- IBMPC production bytes equal current NXVM at this revision. Fixed XT, AT,
  DeskPro and default composition contracts remain unchanged. This import
  does not repair T544's queued CPU gaps.

## Test Coverage Review

420 non-identical paths are inventoried. Of 298 changed/relocated C/H test
bodies, 293 match after only includes, facade names, literal puts/fputs
formatting and fatal fixture assertion substitutions. This is supplementary
to diff/contract review, not a claim inferred merely from passing gates.
Release test registrations retain assertions via -UNDEBUG or /UNDEBUG.

Five non-mechanical bodies were reviewed separately:

1. Audio: controlled idle-wait failure and cancel/join retry regressions,
   independent of physical output volume.
2. Common machine: identical-text publication after reset.
3. Common wait: invalid payload retained but stale/invalid capture rejected.
4. x86 Debug: owner-local driver fake replaces a foreign Common test fixture;
   paused Debug and sole Common executor/thread assertions remain.
5. IBMPC IRET: removes a repeated inclusion of x86 protected IRET. The retained
   independent machine_protected_iret_smoke main executes every removed
   success/flags/fault-atomicity vector plus the conforming-code vector;
   distinct IBMPC real-mode/PIC assertions remain.

45 old-only x86 files have IBMPC receivers. Seven additional owner-local fixture
copies match normalized bodies and remove foreign test-package includes.
44 other files must be retained in NXVM product tests: 38 composition/profile
files plus six Core boot/memory-registration/video-topology fixtures. S2 moves
them to the listed product receivers, preserving assertions and test identities.

Two old Lib scripts move to test root. Supporting inputs are test/README.md,
test/register.cmake and four Types/ownership checker/self-test scripts.
Suite CMake registers moved cases with inward links. Receiving CMake, App
includes, historical static checks and integration fixtures need path repair.

## Receivers And Verification

All four PC Apps consume the changed Lib/Common/x86 paths; no public machine,
debug, media, INI or UI contract change requires a shim. Their existing test
registrations/fixture paths need mechanical adaptation.

MyNES consumes Lib Audio/Common Machine, not x86/IBMPC. No MyNES source,
configuration or snapshot change is indicated; both receiving EXEs and its
complete units still require rebuild/run after Shared implementation import.

Eight fixed-snapshot manifests, all four test Types/inward-ownership checks,
Lib DAG/Types layout/KVM naming and Common/x86/IBMPC corpus gates pass.
Unchanged NXVM baseline full units pass once per width: x64 506/506 in 96.91s;
x86 506/506 in 93.11s. These are baseline, not post-import results.
Independent strict-warning IBMPC configuration and complete
shared-ibmpc-tests build passes; its repository-only runtime aggregate passes
155/155 in 98.09s. This proves the standalone pinned suite, not receiver integration.

## Implementation Gate

S2 may import these exact trees only after retaining all 44 product files and
reconciling every mapped fixture/registration/helper. Exact eight-root
path/content equality, manifests, full receiver units and all affected stripped
x64/x86 artifacts are required. S3 runs all 58 original external integration
contexts once. Owner INI/media/snapshot values stay unchanged.
Audit-only production inputs require no new EXE.
