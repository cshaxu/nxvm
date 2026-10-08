# T546 S20 Scalar, Formula, And Transfer Timing

## Scope And Sources

This receiver reconciles the finite S20 rows in the accepted CPU repair
proposal.  It uses the Intel rows already indexed by the 8086/8088, 80186,
80286 and 80386 timing ledgers; it does not claim board-cycle or host-pacing
time.  The sole successful-retirement selector remains
`src/x86/chips/cpu/cpu_timing_model.c`.

| Source-backed row | Prior behavior | Result |
| --- | --- | --- |
| 80286 `LEAVE` | Selector and manifest receiver used 8 clocks. | Manual-L3 5 clocks from the 80286 ledger; selector and manifest receiver now agree. |
| Taken short/near Jcc | Final EIP inferred outcome, so a taken zero displacement looked not taken. | Derive from pre-execution FLAGS; retained next-instruction term remains owned by the existing 80386 transfer selector. |
| `JCXZ`/`LOOP` compatibility fallback | Shared final-EIP inference had the same zero-displacement defect. | Use its pre-execution count/FLAGS predicate without changing its existing unallocated fallback classification. |
| 80386 VM86 `POP FS`/`POP GS` | Successful real/VM timing route rejected VM86 and fell to one-tick unallocated fallback. | Manual-L3 real/VM row: 7 clocks; protected mode remains 21 clocks. |

## Similar-Issue Sweep

`rg` over the timing selector found all final-PC short-branch checks in the
legacy, 80286, 80386 and retained compatibility paths.  All now use one
pre-execution predicate.  The 8088/80186 manifest runners retain their
existing complete source-formula coverage; no new transfer, odd-word,
width or direction discrepancy was found.  The 80386 timing manifest runner
also passes after the VM86 allocation.

## Direct Proof

- The cross-profile branch smoke executes taken `JZ +1` and `JZ +0` for all
  five retained CPU profiles and requires equal classified timing.
- The 80286 ledger smoke observes taken zero-displacement `JZ` at 7 clocks
  and `LEAVE` at 5 clocks; the full 771-key manifest receiver now agrees.
- The segment-selector smoke executes VM86 `0F A1` and `0F A9`, verifies the
  loaded selector, a classified source result and the exact 7-clock row.

Focused x64 and x86 executions of the three direct smokes and the
8088/80186/80286/80386 manifest runners pass.  Complete dual-width repository
unit suites pass 506/506 per width.  The x64 first pass exposed one isolated
`cpu-bus-boundary-negative` timeout; its direct retry passed, then a complete
quiet x64 rerun passed.  The x86 complete pass passed without a failed entry.

## Receiving Artifacts

All four receiving Apps rebuilt their `0.5.0546` Release pairs from this CPU
revision.  SHA-256 identities are:

| App | x64 | x86 |
| --- | --- | --- |
| My5160 | `39D7C676A505E18229BB6366223A9D817EC2BF966DF6E9118C9549069E5C24B0` | `406CD6FFE7B0E83B7511D1A1B3986A7D3DC839C7FB158755CAA119F5BE6F74DF` |
| My5170 | `E23E9597169B249F97F8D044F2EB523DCAA2C55E211C4A960DD1DC2834EBE249` | `B31067C599EF822C1F928021E75086E8B920343E7FF5550F6DD2FD765194ED00` |
| MyDeskPro386 | `12A0A5D79096454292C01FC2408A00C56B6897323E36A7B4C8D32DD50F7D9FC4` | `D6CF939409D7FF2C7AA3957B2D291A0B408149E9BDFF12CB2DFA9ECA9FBD88FD` |
| NXVM | `D30FFE0C5842D1B9E06EDD51D919D64917653273D904B82F0E9B1A6FFFB31D47` | `CBB8105DB6F5E60AC1BFC35FE2DD23F946073EAF71655E22E4D08C66106C24F4` |
