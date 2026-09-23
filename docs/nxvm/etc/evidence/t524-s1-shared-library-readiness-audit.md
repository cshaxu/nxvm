# T524 S1 Shared-Library Readiness Audit

## Method And Frozen Universe

This audit compares every tracked file below `src/lib/` (except the manifest
itself), every manifest entry, the root CMake library source lists, public
headers, `test/lib` consumers and all source occurrences of prohibited product
or machine vocabulary. It is an inventory, not a declaration that a future
SoftPC binding already exists. The selected source revision is `9ef5e899` plus
the uncommitted T524 planning documents; no library source is changed in S1.

The corpus has 61 tracked library files and 56 manifest entries. The manifest
omits exactly these five tracked host-clock files:

- `host/clock.c`
- `host/clock.h`
- `host/clock_internal.h`
- `host/linux/clock.c`
- `host/win32/clock.c`

It has no entry for a nonexistent path. `test/support/ux_library_manifest.cmake`
verifies listed hashes only: it does not detect missing or extra files, records
no source revision, and is misleadingly named as UX-only although it covers the
current partial library manifest. CMake maintains multiple root-owned source
lists and `lib-neutral-corpus`; `src/lib/` has no standalone CMake entry or
verifier, and no CI workflow exists to reject a stale manifest.

## Component Disposition

| Root | Current useful neutral capability | Confirmed gap | S receiver |
| --- | --- | --- | --- |
| `base` | Fixed-width values, statuses and atomics; no product include. | The corpus verifier must cover it and every private header; no independent source revision is recorded. | S2 |
| `ux` | Copied frame/event values, explicit router, native loops and mailbox are real reusable mechanics. | Linux terminal input maps directly to x86 Set-1; mailbox producer/consumer, destroy and wake rules are undocumented; frame validity is not a checked contract; duplicate action chords silently coexist; full-tree product/machine vocabulary remains. Existing smokes are component probes plus one combined neutral consumer, not two full neutral consumers. | S3 |
| `host` | Clock plus opaque event/task/cancel/join are neutral. Windows and Linux both implement a manual-reset event. | Manual-reset behavior, task destruction, signal/timeout/cancel precedence and UI/executor rules are implicit; no wait-any; Linux timeout uses `CLOCK_REALTIME`; no clock-specific conformance test or cross-platform parity record exists. | S4 |
| `session` | Opaque lifecycle state and callbacks do not own a machine thread or safe point. | `executor` only forwards start/stop around local state. There is one NXVM binding and test probes, but no proof that a second consumer needs this facade; no callback driver is justified yet. | S5 |
| `storage` | Copied byte buffers support readonly, writable and overlay modes; host file mechanics remain neutral. | `DIRECT_*` accepts an owned memory buffer, not a path-backed image lease; commit selects an arbitrary path; replacement, concurrent access and destruction rules are undefined. No second consumer proves which richer persistence contract is shared. | S6 |
| `observability` | Small copied status/sequence outcome has no product dependency. | It has no synchronization/lifetime contract and only one combined neutral consumer plus its direct smoke. No bounded diagnostic event is justified. | S7 |

## Product-Vocabulary Sweep

The required full-tree sweep is currently nonempty. There are no `softpc`,
`mvdm`, `ccpu`, `cvid`, `bios`, `rom`, `profile` or standalone `vm` hits, but
`guest` and `controller` remain in library source/comments. The material
violations are `ux/linux/input.*` and `ux/linux/console.c`, which translate
terminal keys to x86 Set-1 scan codes, and Win32 presenter/mouse identifiers
and comments that describe guest/controller/DIB-specific behavior. These must
be replaced with host-neutral surface/input terminology and a product-owned
key mapping; comments that merely deny a dependency must be rewritten too so
the complete-tree invariant is mechanically verifiable.

## Repair Plan And Completion Predicate

S2 owns the P0 corpus/publish correction only. S3 through S7 each consume the
complete row above; they may retain a capability outside `lib` when the two
neutral-consumer requirement is not met, but may not manufacture a generic
layer to avoid that result. S8 is the only closure batch: it requires the
complete manifest verifier, standalone selected-platform build, CI drift gate,
two neutral consumer/conformance proofs for every retained root, zero forbidden
vocabulary hits, Windows and Linux lib-only evidence, and the normal NXVM unit
and integration gates.

## S1 Verification

- Actual/manifest comparison: 61 tracked files versus 56 entries; the five
  omissions above and no stale manifest entry.
- Source/CMake/public-header/test/vocabulary inspections completed.
- `ctest --test-dir build/mingw-gcc-x64-s8-make -L unit -j 4 --output-on-failure`:
  311/311 passed, real time 23.23 seconds.
- Documentation governance and `git diff --check` pass for the T524 admission
  documents.
