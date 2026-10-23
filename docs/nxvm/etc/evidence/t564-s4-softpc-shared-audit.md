# T564 S4: SoftPC shared-package audit

## Baseline and scope

Read-only comparison on 2026-10-22: NXVM `0ee06e444` and SoftPC
`9ee4af66`. SoftPC's tracked working tree was clean; NXVM's two modified
PC INI files were outside the audit and were not touched. No shared code,
tests, assets, or sibling repository files were changed in this audit.
The source/test pairs for Lib, Emulator, X86 Debug and X86 IBM PC form the
eight audited roots. Comparison covers tracked paths and SHA-256 file bytes.

## Inventory

| Root | NXVM tracked | SoftPC tracked | Changed on both sides | NXVM only |
| --- | ---: | ---: | ---: | ---: |
| src/lib | 123 | 123 | 12 | 0 |
| test/lib | 78 | 78 | 9 | 0 |
| src/emulator | 29 | 28 | 7 | 1 |
| test/emulator | 23 | 23 | 7 | 0 |
| src/x86-debug | 15 | 15 | 0 | 0 |
| test/x86-debug | 9 | 9 | 0 | 0 |
| src/x86-ibmpc | 13 | 13 | 3 | 0 |
| test/x86-ibmpc | 7 | 7 | 2 | 0 |

Counts include manifests. The only NXVM-only path is
`src/emulator/product/composition.h`; SoftPC internalizes that composition
implementation. There are no SoftPC-only tracked paths in these roots.

## Change and quality review

* Lib PCM worker initialization and Win32 Console reader retirement change
  failure handling. If a worker cannot be safely joined, SoftPC takes a fatal
  process-exit path rather than returning with potentially live resources.
  Its console teardown also continues independent cleanup after an error.
  These are observable failure-contract changes, not mechanical refactoring.
* Emulator UI destruction now consumes the UI on every normal return, including
  cleanup errors; Product follows that ownership contract and propagates the
  cleanup error. NXVM's prior contract retained resources on failure. Import
  must use the matching Lib and Emulator versions together and check every
  downstream ownership assumption, including failure-injection coverage.
* SoftPC makes Product composition private and removes checks for impossible
  intermediate states from private helpers. This is a smaller surface, but
  the existing sequential composition and failure tests must remain valid.
  KVM's input-source identity allocator is similarly internalized; inspect
  its remaining overflow coverage rather than reintroducing a test-only API.
* Presentation boolean normalization, IBM PC command simplification and use of
  character buffers for text are local cleanups. X86 Debug is byte-identical.
  Changed tests and manifests must travel with their production changes.
* The Console change replaces cancellation attempts with line completion and
  join, so real Windows Console handoff, cancellation and failure behavior
  require validation. It must not be described as merely a comment change.

No specific reverse dependency or new second production path was confirmed
in this bounded comparison. This is not a line-by-line proof of every file or
a claim that all code-quality problems in the packages have been found.

## Checks and recommendation

All eight SoftPC manifests passed their own validators. Lib Types layout,
Lib direct-component dependency and KVM naming checks, and Emulator, X86
Debug and X86 IBM PC corpus boundary checks ran without reported errors.
The dependency checker was initially invoked without its required root and
stopped; it was rerun with `LIBRARY_ROOT` explicitly set to the SoftPC Lib
directory. No dynamic tests were run as part of this read-only audit.

The committed candidate can be considered for a paired, verbatim import,
but import qualification is not yet established: review the changed failure
and ownership contracts against NXVM/MyNES consumers, then build and test the
complete matched eight roots and affected products on both architectures.
Include real Console/Window handoff verification where available; report
desktop and RDP coverage separately. Do not mix uncommitted sibling content
or selectively combine the old and new cleanup contracts. Import itself is
outside S4's admitted scope.
