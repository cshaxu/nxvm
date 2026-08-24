# T447 S11 independent closure audit

## Scope and method

This is a finite reconciliation of the T447 ledger in
[the approved proposal](../../proposals/m5-architecture-boundary-debt-closure.md).
It does not claim that unrelated TODO rows, future CPU work, physical timing,
or the entire repository are complete. The audit inspected the accepted S1--S10
implementation commits, their current evidence, current source/build guards,
public headers, TODO/Queue, the rebuilt artifact and the full current test
suite.

## Frozen ledger reconciliation

| Original audit row | Receiver and current proof | Disposition |
| --- | --- | --- |
| Td S125 A | S10 P1 `9e05faf8`; one `core_machine` remains while timing, plan, scheduler, firmware, display, board and diagnostics have owner-local files. | Accepted. |
| Td S125 B/I; Td S127 C; Td S129 A | S2 P1 `52d140a8`; `verify_t447_session_boundary.cmake` proves one SESSION grammar, no selected raw borrow and immutable tokens. | Accepted. |
| Td S125 C/D; Td S128 A--C; Td S130 A | S7 P1 `b2bf42da`; private VM layouts, duplicate Model-40 constructor, reverse adapter, KBC-private vocabulary and stale includes are absent. | Accepted. |
| Td S125 E; Td S126 A | S5 P1/P2 `13f2a5bc`/`af805310`; platform lifecycle guard proves opaque owner-created state. | Accepted. |
| Td S125 F | S9 P1 `e44f3369`; `verify_t447_vdm_forwarding_closure.cmake` rejects the former VDM facade and target. | Accepted. |
| Td S125 G | This S11 record limits the conclusion to the named T447 ledger and retains the closed audit as supporting evidence. | Accepted; no broader claim. |
| Td S125 H; Td S127 D | S8 P1 `acf3f7cc`; `verify_session_layout_boundary.cmake` rejects cross-owner product/platform private-layout tests. | Accepted. |
| Td S126 B/C | S6 P1 `e342a516`; collaborator/plan guard proves copied declarative plans and Core-owned endpoints. | Accepted. |
| Td S126 D | S3 P1 `d67c4892`; debugger guard proves one opaque interpreter authority. | Accepted. |
| Td S126 E; T446 exception | S4 P1 `38f5097e`; file-resource guard proves one synchronous Core owner and no recorder direct-source exception. | Accepted. |
| Td S127 A/B | S1 P1 `9a81283d`; build-ownership guard proves unique source target ownership and adapter-native links. | Accepted. |

The 15 stale TODO entries corresponding exactly to these accepted receivers
were removed in S11. No `T447` receiver remains in TODO or Queue.

## Current-source and build-graph review

- Core has no `#include "vm/"` or `#include "vdm/"` edge; the dependency-DAG
  specialized guard reports zero migration allowlist edges.
- `machine.c` is 1,090 lines, owns the sole aggregate and lifecycle path, and
  calls each owner-local mechanism directly. There is one Core execution path
  and one transaction path. The S10 rollback inventory confirms port and ROM
  rollback remained single-path.
- Current guards name the actual source owner instead of preserving a false
  `machine.c` location requirement: board, display/plan, scheduler, firmware,
  and CPU timing-model checks all passed.
- No accepted T447 source file, public-layout compatibility alias, VM/VDM
  reverse dependency, obsolete VDM forwarding target, or direct-production
  test exception was found in the current changed paths.

## Per-S code accounting

Tracked implementation commit accounting, excluding P acceptance records and
this S11 audit: S1 `+159/-31 net +128`; S2 `+521/-273 net +248`; S3
`+181/-59 net +122`; S4 `+444/-598 net -154`; S5 P1/P2 `+644/-365 net +279`;
S6 `+746/-453 net +293`; S7 `+1094/-736 net +358`; S8 `+79/-20 net +59`;
S9 `+123/-161 net -38`; S10 `+6337/-6109 net +228`.

Positive nets are bounded owner-private representations, copied observations,
focused guards and failure contracts added while duplicate layout/path code was
removed. They do not introduce a second production path. S10's apparent
positive total is source relocation plus its 85-line evidence; its former
6,786-line coordinator is now 1,090 lines.

## Verification

- Strict full MinGW build passed.
- `verify-current-specialized-gates` passed all 77 gates, including T345's
  intentional negative self-test.
- Full CTest passed 292/292, zero failed, in the authoritative final
  `LastTest.log` (300.98 seconds).
- Documentation governance and `git diff --check` passed before S10 P1/P2;
  S11 repeats documentation governance after its record and TODO cleanup.
- Current artifact SHA-256:
  `8279CA97F977C40EDDF014D03DB21E1A89839E602E81F7D4829D6A14E1BF2601`.
