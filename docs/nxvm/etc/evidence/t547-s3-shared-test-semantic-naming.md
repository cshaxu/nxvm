# M5 T547 S3 — Shared Test Semantic Naming

## Scope and disposition

- `test/lib` and `test/common` were scanned and contained no current
  task-derived filename, registration, symbol, comment, or result marker; they
  remain source-identical.
- `test/x86` and `test/ibmpc` rename their current test identities to the
  behavior they own.  No assertion, fixture ownership, test registration
  domain, production source, or public interface changed.
- Historical M5/T/S identifiers remain in proposal/history/evidence documents
  only.  They are absent from the four current shared test source and
  registration trees.

## Required 80386 identity

- Source: `machine_80386_secondary_integer_timing_smoke.c`.
- Target: `machine-80386-secondary-integer-timing-smoke`.
- Private symbols: `secondary_integer_timing_*`.
- Success marker: `80386:SECONDARY-INTEGER-TIMING:OK`.

This test covers the 80386 secondary integer timing group: near conditional
branches, BT/BTS/BTC, SHLD/SHRD, MOVSX/MOVZX, BSF/BSR, IMUL and prefix/invalid
LOCK paths.  It is an x86 Core test, not an IBM PC board or App test.

## Shared test infrastructure

`test/register.cmake` is explicitly normalized as LF through
`.gitattributes`, matching the eight shared source/test corpora.  This changes
no CMake behavior.

## Verification

- Static task-identity scan of all four package source/registration trees:
  no current task-derived identifier remains.
- x64 and x86: the three directly renamed owner targets built and passed,
  including the required 80386 secondary integer timing target.
- x64 and x86: all 19 shared manifest/corpus/types/boundary checks passed.
- Full package executable aggregates are deliberately not claimed by this
  evidence until their complete dual-width build/run finishes; no passing
  result is inferred from these focused checks.
