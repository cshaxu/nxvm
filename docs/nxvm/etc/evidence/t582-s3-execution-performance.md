# T582 S3 — execution and memory-route costs

## Audit and bounded design

Baseline is S2 delivery `d8a94dc61`, `7c7a98a13`, `0d48682cf`, with S3
admitted at `1f4e0c641`. Task-owned instrumentation linked the current Machine
memory implementation into the real PC110 public-factory receiver. It ran on
the guest worker and reported only after shutdown, not through unsafe live
cross-thread inspection. Owner CMOS/media were copied; disks used overlays.

In a 20-second x64 startup sample, 41,333,796 route resolutions included
27,158,322 RAM outcomes and 13,502,695 replacement-provider outcomes.
There were no aliases or active address-clear bits in that observed phase.
Cache-fill checks were 390,085, of which 68 enabled fills. Counts are not cost
percentages; instrumentation changes speed. The earlier supposition that
aliases explained the rejected S1 candidate is not supported by this phase.

The existing code resolves a wide RAM request and then resolves each byte
again to detect narrower provider windows. Cache-line admission similarly
checks every byte. A private geometric proof skips these repetitions only
when there are no address-clear bits, no mappings, no physical wrap and no
registered provider intersecting any byte. It is conservative for CPU-only,
declining and fallback providers. All other cases retain the existing logic.
Cache admission still validates installed backing capacity and identity.

Machine also copies CPU timing repeatedly after the same refresh. The
intervening predicates and prefetch reservation do not modify timing_result.
One local copy preserves short-circuit fault-consumption behavior; pending
retirement handling keeps its separate existing read. No persistent cache,
public contract, guest clock or executor changes.

## Preliminary fixed-work comparison (superseded)

An existing Core run wrapper caps the final instruction budget at the remaining
work and records the exact completion before publishing an atomic done flag.
The normal control thread stops the Machine. Only the measured interval ends
at the checkpoint; no instruction is replaced or skipped. Each row executes
10,000,000 instructions, ending at PC `0003dbc5`, ticks `79,964,294`.

| Width | Baseline ms | RAM proof ms | RAM proof + one timing copy ms |
| --- | ---: | ---: | ---: |
| x64 | 22,636 | 21,565 | 20,903 |
| x86 | 27,861 | 26,032 | 25,119 |

These preliminary measurements motivated the candidate, but final review found
that directly compiled probe sources omitted the Release trace-disable define.
They are not valid receiving-performance evidence. The final matched Release
comparison below supersedes their percentages. Production builds and tests used
the correct configuration throughout.

## Admitted implementation

Modify only `src/x86-core/machine/memory.c`, `machine.c`, the existing
`test/x86-core/machine/machine_memory_device_registration_smoke.c`, and their
two manifests. Estimated production +30/-8, tests about +35/-0. Keep the
existing registration fixture and test wide RAM, a one-byte provider inside
a wider read/write, cache admission next to and across that provider, and
installed-memory bounds. Existing alias, CPU overlay, parity/snoop, cache and
executor event regressions remain the owning coverage; do not duplicate them.

Before S closure require actual diff review, full cached dual-width tests,
all fourteen receiver artifacts and a representative product comparison.
No manual RDP or complete performance qualification is established here.

## Implementation status

The admitted implementation is now in the worktree: production +31/-10 (including the corrected cache-policy comment),
existing regression +23/-0, plus manifest entries. The physical-range check
uses a subtraction bound to avoid overflow in its own wrap check. Existing
per-byte routing remains for ambiguous spans. RAM parity and snoop delivery
still execute after the unchanged backing copy; they are not bypassed.

Both architectures build and pass the actual memory-device-registration
CTest case. The new assertions exercise plain two-byte RAM, a two-byte access
containing the existing one-byte provider, adjacent versus intersecting cache
lines, and the first address beyond installed memory. No additional test
executable or fixture was introduced. The complete receiving build/test matrix subsequently passed as recorded below.

## Follow-up hotspot review

Read-only review distinguishes chip DMA advancement from board DMA arbitration.
The chip already returns immediately when idle. Board-Base's
`x86_nxvm_board_dma_advance_transaction` instead repeats arbitration and cascade
refresh for each elapsed DMA tick, including idle intervals. This is a candidate
for measurement, not an admitted optimization. The existing S1 samples
include 134 early and 101 late samples in board DMA advancement, and 103 early /
117 late in chip signal reads. These are sampling observations, not percentages
or proof that idle batching is safe or worthwhile.
Any batching must preserve cascade propagation, grant/transfer boundaries and
device callbacks that can change requests; merely checking the initial pending
flag is not sufficient evidence to skip the loop. Keep this investigation out
of the frozen S3 implementation while its receiving qualification runs.

## Actual-diff review during qualification

The timing snapshot is initialized after CPU refresh on every path that uses it;
retirement-wait paths retain their own read and jump past the new local use.
`execution_reserve_prefetch` changes prefetch reservation fields, not the timing
result. The capture accessor itself only returns the existing result.

The cache-policy comment describes the conservative whole-span proof plus
byte-query fallback. Completed receiving results are recorded below.

## Completed receiving qualification

The background qualification script exited zero and reported
`FULL DUAL-WIDTH QUALIFICATION COMPLETE`. Each width passed 712 unit tests,
48 registered integrations, and the three additional profile selections
(XT 3, AT 5, DeskPro 6). Stable graphs were restored to the default profile.
No failure or skip was converted into a pass. The documentation gate and
`git diff --check` also passed. External selected-set receiving checks also passed below. Performance evidence
is separate from functional qualification.

All fourteen current artifacts were verified as the expected PE architecture.
MyRaidenII was deployed from the current App target and stripped with the
matching toolchain. MyNES was built and verified; unchanged bytes need no
artificial binary edit. Owner INI, CMOS, media and snapshots remain unchanged.

| Artifact | PE | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/nxvm_0_5_0582_x64.exe` | x64 | `A74E8AC1345A450D473D24B701E39447B800567FDB0F78103CE7F66D0A650830` |
| `assets/my5160/my5160_0_5_0582_x64.exe` | x64 | `5D8F1985ADC29479CCF66E5FC23A270BC632D69D37D305D89F1D476AB53E8458` |
| `assets/my5170/my5170_0_5_0582_x64.exe` | x64 | `13A46A7D169171D5138127EFCAA226156C3F90CB121FBC39C6D146A5B051CE5A` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe` | x64 | `8691283C7D69036930A046B72440C434515C72421C7A3CF71C6EBDA0B37A4C2A` |
| `assets/mypc110/mypc110_x64.exe` | x64 | `72C464851252765EA1AEF1A0DCDC7081A2EE165340E9A0196F50B827DF41418E` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | x64 | `3EE74F9CFAD3A4D207806D09B15F501CD0AE3AA8749717C729D2893263B92B01` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | x64 | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| `assets/nxvm/nxvm_0_5_0582_x86.exe` | x86 | `9A1626C4431CD9785A791EBDE4512E1E652AB860B9A0A90DD9A6165525D4314C` |
| `assets/my5160/my5160_0_5_0582_x86.exe` | x86 | `E17CDF0C1CAE6340417D78C4C3FCB4FB51E465C03C20F8C5065E8C79BDB134AF` |
| `assets/my5170/my5170_0_5_0582_x86.exe` | x86 | `D423DC4B28978044E07F30C62A811033F48667F1F9E96DE79275D87302A6B4E0` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe` | x86 | `41FC80E595DF5540E07A853331D0A7F51ACDE21704894D05A0439F8867F5F938` |
| `assets/mypc110/mypc110_x86.exe` | x86 | `F72FCCE7AD0A0BD8038669A508E79BDACC0641E3AF4345C5E50EAA87A5C65B88` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | x86 | `E645F83A291BEC3519E719C4E97362E2A487D3A9B8D5BE9474D7016F4A47DCF5` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | x86 | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |

Qualification log SHA-256: `FEEB38F174DA250C8A155E7519F0BB0E14ABF7B4746587A70FE389FBCEB46EA9`.

External receiving checks now pass on both widths: selected-set startup retires
2,000,000 instructions at 33,287,472 ticks with frame digest `db1f73fa40511ae0`,
and each deployed MyRaidenII product passes Console control. These preserve
startup evidence, not gameplay/audio qualification.
External log SHA-256: `D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`.

### DMA candidate boundary clarified

Core arbitration deliberately calls the board even when its pending-request
query is false. That call also maintains cascade state. In addition, the
board's advance function can continue a primary active channel while the
pending query is gated by the secondary cascade request. Therefore an early
return based solely on `dma_has_pending_request()` is not an equivalent
optimization. Reject that shortcut. A future measured idle proof must examine
both controllers' actual request/active facts and preserve the existing cascade
update before skipping remaining ticks. Existing `board_dma_channel_smoke`
already covers masked cascade, reenable, disabled controllers and real data
transfers, so use that owner instead of adding a parallel fixture.

## Rejected unequal-build product comparison

The preliminary 300-second x64 pair reported baseline 766,587,180 sampled Core
ticks / 142,625,281 instructions, candidate 748,987,942 / 139,694,538; both
first matched the complete Personaware menu at sample 27. The baseline final
image was visually inspected. These results do not establish a speedup or a
regression: review of actual compiler commands found that directly compiled
baseline machine.c defaulted runtime trace to 1, while the candidate archive
correctly used Release runtime trace 0. The x86 pair was stopped after verifying
and terminating only its owning task process and child. No product or owner
process was terminated. No trace policy or production configuration was changed.

The replacement comparison compiles both baseline and candidate memory.c and
machine.c directly, with identical -O3 -DNDEBUG -std=c11 and
-DX86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=0, the same compiler and link graph.
It executes exactly 30,000,000 instructions, with the same private CMOS/media,
sequentially per width, and compares PC and guest ticks as well as host time.
The budget is 150 seconds per probe. This measures a bounded early guest
workload, not native Window/RDP or full playability. Record the actual command
with each probe; future task probes must copy relevant target definitions,
not only optimization flags and libraries.

### Additional CPU candidate, not implemented

The 486 source-cost function counts prefixes before calling nominal, memory,
decode and bus cost helpers, which have further prefix-count call sites.
The counter scans the completed instruction bytes through the existing prefix
classifier. This is a plausible repeated-work candidate, not a measured saving.
If pursued, prefer passing already computed local facts within the calculation
rather than adding a second persistent decoded-state cache. Preserve separate
preview/runtime callers and V30 emulation semantics. Do not relax any timing
formula or claim this work has been implemented in S3.

## Final review scope

Production is +31/-10 (net +21), existing test +23/-0 (net +23),
counted with git diff --numstat on the three C files; the two manifests
replace three hash entries. One private span-proof helper owns its geometric
condition. Ambiguous routes retain the original per-byte resolver; reset/raw,
provider, A20, aliases, backing bounds, parity, snoop and fault paths retain
their existing owners. There is no new public interface, state, thread, clock,
queue or cache. The timing copy remains local to one CPU refresh.

The tests add no fixture or executable. Full test success establishes the
registered regression gate, not all possible hardware states or manual RDP.
Native Window responsiveness, interactive gameplay and realtime guest clocks
remain unqualified. No owner INI, CMOS, disk or snapshot was modified.

## Matched Release measurement and disposition

| Width | Baseline ms | Candidate ms | Host-time reduction |
| --- | ---: | ---: | ---: |
| x64 | 65,712 | 62,708 | 4.57% |
| x86 | 79,759 | 74,483 | 6.61% |

All four runs retire exactly 30,000,000 instructions at 195,624,840 guest
ticks and PC 000f6440, then stop/shutdown normally. Both variants use the
same Release definitions and private inputs. Retain the bounded local repair
on this equivalent-work evidence and completed regression qualification.
This single sequential pair per width is not a variance study or a complete
startup/steady-state speed claim. T582 remains open: nominal realtime and
interactive playability have not been established. The next measured owner
is board DMA idle arbitration, subject to preserving cascade semantics.

Measurement log SHA-256: `63D3F119C9BA86F176DEA012BC0A545E878E55079223F77FCB93895D39A5EAC5`.
