# T3 S9: Selected-Set COP Initialization Protocol

## Baseline And Method

The tracked production/test baseline is accepted S8 `2a6e13b8d`. No tracked
source, test, CMake, Shared contract or artifact changes in S9. The owner
delegates same-T ordering. This consumes the initial B07/B08 protocol batch,
not the full game's operation universe or T3 completion predicate.

A disposable diagnostic copy of the current COP adds one observer immediately
before its unchanged explicit command rejection. The probe links that copy
and the current Board/Video archive against the already available observable
X86 Core library. It creates the actual configured V30 Board and uses its
one public run/trace route. It neither accepts an unsupported command nor
restores S6's old RAM fallback. Only neutral command identifiers, counters,
widths and upload-address statistics are emitted; uploaded program contents,
register values, ROM bytes and raw instruction traces are never logged.

The external archive SHA-256 remains
`9FEDC8DDAC2208408C3B05201316E3F2DB43BDFB104551BA17337BB55DB13D71`.
Only two lanes were extracted, each 524288 bytes, and verified before use:

| Lane | SHA-256 |
| --- | --- |
| program-even | `90BB05525C1529B97C359DEB47305E8E25EE9DE8D7C951B0BE0B5587C89354EC` |
| program-odd | `C0A6E9A660843851F1CFBCFA84977F82F69A2FD7A4B28B7561C692389D4577B7` |

The containment budget is 2M instructions, 120 seconds and a fixed summary
under 64KiB. The run stops earlier at the actual first rejected command.

## Direct Observations

| Fact | Measured result |
| --- | --- |
| Retired instructions before failure | 68254 |
| Core elapsed ticks | 1076826 |
| Returned status/reason | INVALID_STATE / FAULT |
| First and only command attempt | `0x5205` |
| COP write width | 434 word writes, zero byte writes |
| Upload address/data writes | 176 each, 176 distinct addresses, maximum 247 |
| Data without a preceding address write | zero |
| Parameter/mask/trigger writes | 22 each |
| First command ID already among uploaded triggers | yes |
| DMA source/size/destination/mode writes | two each |
| Main interrupt acknowledgements | zero |
| CPU IF at the rejection | clear |
| Next Video deadline | 1112064 |

The Video model has progressed past three VBlank edges, but IF remains clear
and no acknowledgement occurs. This rules out explaining this *first explicit
command failure* by a missing periodic source. It does not establish why S6's
later old-fallback baseline stops making progress.

The complete preceding write counts by register are:

- `0x432`, `0x434`: 176 each;
- `0x438`, `0x43a`, `0x43c`: 22 each;
- `0x478`, `0x47a`, `0x47c`, `0x47e`: two each;
- `0x4a0`, `0x4a2`, `0x4a4`, `0x4a6`, `0x4c0`, `0x4c2`, `0x4c6`: one each;
- `0x500`: one rejected attempt.

S4's `0x428`, `0x42a`, `0x42c`, `0x444` and `0x476` configuration writes
are absent from this strictly earlier prefix; S4 observed them only in its
longer RAM-fallback execution. They remain in the original B07/B08 universe,
not removed or declared unused. This probe instruments accepted COP writes
and the rejected command; it does not claim new result-read or DMA-trigger
coverage from an observer that did not measure those families.

## Finite Initial Trigger Ledger

The 22 identifiers below are observed uploaded hardware command identifiers,
not copied uploaded program words. Upload does not prove subsequent execution.
Only `0x5205` is actually attempted by this strict run. The pinned MAME COP
reference is used solely for L2 family navigation; it supplies no imported
implementation or table, and its incomplete cases are not hardware proof.

| Observed IDs | Candidate receiving batch | Present disposition |
| --- | --- | --- |
| `5205`, `5a05`, `f205` | S10: protocol and memory-transfer family | First ID attempted; all three uploaded. Exact operation/bus/failure contract must be frozen before code. |
| `0205`, `0905`, `2a05` | Later B07 numeric/state-update batch | Uploaded, not executed; exact model remains required. |
| `130e`, `2208`, `338e`, `3bb0`, `42c2`, `4aa0`, `6200`, `8100`, `8900` | Later B07 geometric/numeric batch | Uploaded, not executed; independent numeric/edge oracles and explicit model grade remain required. |
| `a100`, `a900`, `b100`, `b900` | Later B07 collision/query batch | Uploaded, not executed; no fabricated result is admitted. |
| `1905`, `9100`, `9900` | Later B07 semantic reconciliation | Uploaded with no established executing meaning; retain explicit rejection until qualified, not a no-op. |

This extends S6's family/count evidence into a bounded *initialization*
ledger. Later code may upload another program or issue another form; such
evidence refines the original B07 ledger before implementation, rather than
silently declaring these 22 IDs the full playable-game coverage universe.
B08 DMA trigger/result behavior remains unobserved and unresolved. S10 must
preserve explicit failure for every non-admitted operation.

## Next Implementation Receiver

S10 receives the entire upload/register protocol and the three-member memory
transfer family. It must define stored runtime upload data, register lane
semantics, reset, operation admission and invalid/incomplete input behavior
without embedding the game-uploaded program or matching a game fingerprint.
Memory effects use the existing public Core transaction/DMA route. The
command arrives inside an active CPU write transaction: implementation must
audit the safe scheduler service point rather than recursively beginning DMA
inside that transaction or using paused-debug reads to bypass bus ownership.
No new Shared interface is admitted by this research record.

The test receiver extends the existing Board unit for this cohesive protocol
and transfer batch, including actual guest-issued commands and failure/reset
cases. It does not create a second CPU fixture, per-command test executable or
generic COP interpreter without a separately evidenced instruction contract.
Reference-derived functional behavior remains L2; synthetic vectors need
independent expected memory effects. Original COP documentation is desirable
but absent documentation is not permission to copy reference algorithms or
to fabricate success.

## Identities, Verification And Cleanup

The only external source inspected is the previously pinned MAME COP file at
commit `c1425a1af00b88aa07477b1b23a30ed5e5f856ff`, SHA-256
`7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9`.
Its dispatch declarations/comments support L2 navigation only. No third-party
code, program table or operation body is copied into this repository.

| Disposable item | SHA-256 |
| --- | --- |
| Final diagnostic source | `07E8BDAEA8889E9A5D6FBE5F49E2B42B847611F0395E466ABAFB50144DB8ED28` |
| Instrumented local COP copy | `84E0C78D6CBDE713412AD59536D024BF7B7F67CF24DE22B937FAC9158376D69F` |
| Diagnostic executable | `8A7F9C707F0229E64894C811D686900F58F512FC6F43C853519FD822F956DDDF` |
| Final 1232-byte neutral summary | `1DE3AFA31CDDD5CC056A0272786EA3E4469B7FA5EBCC0AF9C82A33C7EE884F4A` |

The probe passed strict C11 compilation and the bounded runner completed with
the expected single actual command fault. Earlier probe refinements added
only address/trigger/IF aggregates; they did not change execution behavior.
Tracked production/test/build delta is zero, so S8's exact 687/687 dual-width
source qualification remains applicable, not a claimed new suite execution.
No EXE release, desktop, RDP, external complete-suite or game boot pass is
claimed. Documentation and actual-diff review apply to this evidence delivery.

The task-owned root contains ten files totalling 1788292 bytes, including the
two verified extracted copies. Every owned process has exited. Required input,
source and summary identities are retained here before governed cleanup.
Preview and apply then removed the entire ten-file root; the external archive
and persistent caches remain unchanged.
