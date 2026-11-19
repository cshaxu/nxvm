# M5 T587 S4 - SoftPC eight-tree quality audit

## Admission and frozen scope

Owner admitted this audit on 2026-11-18 after S3 acceptance. NXVM baseline:
`377a11722`. SoftPC committed baseline:
`4cd56d018abd5328f0578dbfe0b7524f4bcfcce4`; its worktree was clean.
The earlier accepted import is `dfe955eb885e66c5549b6cd7823bb262af83ab69`.
Read all changed lines and the relevant unchanged parsing, copying, routing and
receiving code. Reviewed the eight package inventories, names and checks.
No sibling edits, source import, binary refresh or product version change.

## Decision

The audited shared source is eligible for byte-exact import with receiving
callback migration in the same implementation batch. It is NOT a drop-in
replacement for the current receiving source. No confirmed new Shared defect
requires a local package fork. This is source/static eligibility, not full
receiving build, runtime or user-experience qualification.

| Tree | Changed files | Added | Removed |
| --- | ---: | ---: | ---: |
| src/lib | 0 | 0 | 0 |
| test/lib | 0 | 0 | 0 |
| src/emulator | 4 | 68 | 45 |
| test/emulator | 2 | 55 | 19 |
| src/x86-debug | 3 | 16 | 4 |
| test/x86-debug | 2 | 9 | 4 |
| src/x86-ibmpc | 4 | 16 | 10 |
| test/x86-ibmpc | 2 | 15 | 11 |

Total: 17 files, +179/-93, net +86, including README and manifest rows.
Production C/H: +78/-51, net +27. Test C: +76/-31, net +45.
No files moved or new subsystem, thread, persistent state or dispatch route added.
Lib and its tests are unchanged, including the previously accepted Console fix.
The Built on CMake refresh and serial-test budgets remain unchanged.

## Contract, structure and failure audit

Emulator Product now owns one whole-line tokenizer. Fixed callbacks receive a
counted `emulator_product_monitor_arguments`; extensions receive command name
plus that list. The old public raw-tail parse function is removed. Storage is
local and explicitly borrowed for synchronous dispatch only. No consumer may
retain token pointers for asynchronous snapshot/media work.

The 1024-byte local line accepts at most 1023 bytes. At most 512 one-byte
words fit with separators; empty quoted words consume more input, not less.
The 512 pointer slots therefore suffice for all accepted input, including
malformed input before it is rejected. Parsing completes before any callback or
lifecycle dispatch. Bad quote placement rejects the whole line. Eight fixed
commands still reject arguments; save/load parameter meaning remains with the
consumer. Fixed lookup precedes extensions, so extensions cannot override it.
Space/Tab separators, whole double quotes, empty quoted tokens, no escaping
and byte preservation are explicitly documented; CR/LF and other control bytes
are no longer generic separators. Console normally supplies an unterminated
line without CR/LF; direct callers must obey the documented grammar.

X86 IBMPC forwards the tokens through its existing single provider. These
forwarders are necessary PC binding callbacks, not a second parser. New types
use Lib vocabulary. Project names remain shared-lib, shared-emulator,
shared-x86-debug, shared-x86-ibmpc and their shared-test-* counterparts.
No App, platform or outer-package dependency is introduced by the delta.

X86 Debug N reuses existing scanlist for quoted filenames; its independent DOS
DEBUG grammar is retained. The command buffer and temporary byte array are
both 256 bytes, and quoted decoding cannot grow the input, leaving terminator
space. Invalid/empty/unclosed names fail before checked filename copying;
checked copying validates capacity before writing. No previous filename is
lost on these failures. This is a local existing-parser extension, not a
second Monitor tokenizer or Storage path implementation.

## Tests reviewed

Monitor smoke uses real submit_line for fixed and extension quoting, UTF-8,
empty arguments, multiple arguments, malformed quotes, maximum accepted input
and overlong rejection; existing fixed priority/lifecycle tests remain.
X86 IBMPC smoke verifies quoted snapshot and floppy parameters traverse the
production provider. Debug machine smoke verifies quoted UTF-8/space filename
through real N/W/L and Storage, with malformed names leaving the previous file
selected. It replaces the former bare-filename storage example; an additional
bare-name example would be useful but is not a demonstrated import blocker.
No new test framework or external product fixture dependency is introduced.

## Required receiving migration

- `src/x86-nxvm/product/factory.c`: migrate shared PC info/speed/floppy
  callbacks to name plus arguments, remove old line normalization/tokenization;
  retain state/mode validation and copy values that outlive dispatch.
- `src/app-mypc110/product_binding.c`, product_card and product_vdisplay with
  their declarations: migrate the existing extension chain. `vdisplay=dstn`
  remains one command-name token and must retain stopped-only semantics.
  PC-card syntax has a literal pipe separating two paths: preserve the
  declared command capability and explicitly test path spacing/quoting rather
  than blindly rejoin tokens and create another raw parser.
- `src/app-mynes/product/command.c`: migrate ROM and save/load callbacks;
  retain actual operation/state validation, path restrictions and copies for
  deferred work. NES Debug stays on its own existing command path.
- Update direct callback fixtures in test/x86-nxvm/product/factory_smoke.c,
  MyPC110 unit/integration extensions and MyNES command tests. Exercise the
  real Monitor entry for syntax; avoid duplicating its tokenizer tests.
- MyRaidenII registers neither changed submit callback, so no matching source
  signature migration is required; it remains a full-build/test receiver.

Do not cast incompatible function pointers, keep deprecated callback aliases,
or reconstruct raw strings solely to retain the old parser. The shared eight
trees can stay exact; X86 NXVM and App receiver changes belong outside them.
All fourteen products require the normal cached full publication and testing
when implementation is admitted. This S does not authorize that import.

## Verification boundary

Static verification completed successfully (details below). No new receiving
build, unit/integration execution, manual desktop/RDP or game-play claim is
made in this audit. Earlier S3 tests cover the old baseline, not this new API.

## Completed static verification and review

All 296 committed files were compared with the upstream worktree and current
NXVM counterpart: upstream bytes equal its fixed HEAD; exactly the 17 paths
above differ from NXVM. All eight manifest path sets and SHA-256 values agree.
The eight native manifest verifiers passed, including header, sorted/unique
paths and LF checks. The remaining 13 checks passed: Lib Types layout/DAG,
three production corpus gates, four test dependency gates, four test Types
gates and Lib KVM naming. Total 21 checks, zero failures in the final results.
Git diff --check passed. Upstream remained clean at the fixed revision.

The initial supplemental Python digest comparison compared lowercase hex with
canonical uppercase manifest hex and falsely flagged hashes. Correcting that
comparison and running the real manifest scripts resolved the audit-tool error;
no source or manifest was edited. Two exploratory rg glob/path queries and one
misspelled check-script read were corrected; these were not product failures.

Coordinator actual-change review: only NXVM S4 status and this report changed.
No new P source/artifact requirement is created by this read-only audit.
Necessary inventory and checks are recorded here; owned scratch is removed
with the existing cleanup tool after the audit process exits. S4 remains open
for owner review; T587 is not closed and import is not admitted.
