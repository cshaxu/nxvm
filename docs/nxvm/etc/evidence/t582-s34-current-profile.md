# T582 S34 — current PC110 profile

At Shared `c1b6f95b4`, a task-only x64 product-path executable sampled thread
instruction pointers during host seconds 5–25 and 220–240. The executable and
configured CMOS were in the same private directory, with private disk copies.
The worker recorded 3,440 early and 3,432 late samples; asynchronous thread
suspension and symbol lookup perturb execution and do not supply inclusive
function costs or cross-architecture results. Other threads had context-read
errors and are not used in the ranking. The sampled run identified a game
frame, but the diagnostic uses the earlier insufficient two-pixel frame check;
this is not a fully rendered-frame or native/RDP qualification.

The named CPU (`x86_core_cpu_*`, `Exec*`) buckets account for 1,067 early and
1,124 late samples; named Core Machine buckets 873/486; named NXVM board/chip
buckets 600/859; `msvcrt.dll` 76/85; all other labels 824/878. These are
exclusive instruction-pointer labels, not complete component attribution:
helpers, inlining, other modules and unknown labels remain in the last group.
Early `memory_route_resolve` accounted for 189 samples; late `pic_select`
accounted for 117. No single named leaf is a demonstrated explanation for the
whole order-of-magnitude product gap. Sampler / guest log SHA-256:
`7BCFAB089BB619923CED851199C748540E49CCB783B3D6BCBD0287B1DC1039AC` /
`2F9C4513AC1E1ADEE315D97F61EA2B25AA54669B798C193E69F7994AE7A18A9C`.

## Rechecked one-pass memory route candidate

The [S15 experiment](t582-s15-cpu-line-audit.md) merged raw-provider inspection
and ordinary-window matching into one pass while retaining raw, selected alias,
replacement, ordinary and fallback query priority. S34 revisited it because
current sampling still identifies memory routing, and S31 has since changed
the existing decoded-address handling. The old candidate's route was adapted
to current source without changing the new fallback decode; it is scratch-only.
It removes five net source lines and a provider traversal, but performs some
matching earlier, including on paths where a selected alias later wins. The
observational-query contract and all error, overlap and mapping semantics
would require direct qualification before any implementation.

With each executable and configured CMOS seed in the same private directory,
all sequential baseline/candidate/candidate/baseline runs reached 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`:

| Architecture | Baseline | Candidate | Candidate | Baseline |
| --- | ---: | ---: | ---: | ---: |
| x64, ms | 20859 | 21270 | 21521 | 21088 |
| x86, ms | 24454 | 24944 | 22012 | 21908 |

Both orderings slightly favored the baseline, despite host variation. Combined
with the earlier late-workload result, this does not justify changing the
production route. It is not a proof of complete state equivalence, a precise
regression estimate, or a claim about other machine configurations. Log
SHA-256:
`722DB9EF3C58456AD134F8C01233BD9211FA15ABDA38B46AA7C07B6A6DA237A2`.

A follow-up scratch candidate computed the decoded-window range match once
per provider and reused it for a raw query when raw and decoded addresses were
equal. Other raw addresses still received their own range check, and query
ordering remained unchanged. It changed 23 lines in and 26 out compared with
the current source, but made the loop's conditions more complex and still did
matching before a possible selected alias. Configured-seed sequential
baseline/candidate/candidate/baseline samples were 22621/22294/23101/22154 ms
on x64 and 25509/26364/27407/27616 ms on x86; each reached the same fixed
instruction/tick/PC checkpoint. Both widths changed direction between pairs,
so no stable gain is established. The small net line reduction does not justify
the increased conditional complexity and potential extra work on alias paths;
this candidate was also rejected without production changes. Log SHA-256:
`8766087A1B2EA853CC106E87A484B20831DE61970FB138F45F82BD2344D76A5F`.

## PIC input-level zero check

Board PIC refresh derives each controller's input bitmask from eight existing
source counts. A configured-CMOS, task-only fixed-work counter observed
20,579,338 derivations on x64, of which 11,674,883 had all eight counts zero;
x86 observed 20,591,174 and 11,680,801. This is approximately 57% on this
startup checkpoint, not a late-game frequency or exclusive cost estimate.

A scratch candidate used a byte-safe copy into an eight-byte scalar solely to
test for all zeros before the existing loop; zero detection does not depend on
byte order. It introduced no mirrored mask or other persistent state. At the
same 10-million-instruction / 79,964,294-tick / `0003dbc5` checkpoint,
baseline/candidate/candidate/baseline timings were 19219/18878/19734/21341 ms
on x64 and 24393/25074/26774/26637 ms on x86. The x64 samples favored the
candidate with substantial baseline variation, while both x86 samples favored
the baseline. The additional branch and copy have no proven dual-width or
product benefit, so no production change or semantic qualification was made.
Counts and comparisons remain startup-only observations, not complete-state or
interactive evidence. Log SHA-256, count and comparison respectively:
`8B2BD9F1CA4CE53A5A9D5C5B7228A6EE200E643C0754DB8FBE316ECE1065B552` /
`A0DD4D27FDA5B83783FCECC3BF149A8F6C376E8E467A2803BDED5EF368B9CEEB`.

## Instruction timing-fact reset

`ExecBoundary` individually resets a contiguous group of per-instruction
timing facts. A scratch candidate replaced those assignments with one bounded
zero operation from `timing_fetch_bus_ticks` up to, but not including,
`memory_lock_active`; the preceding previous-instruction fields also remained
outside the interval. The scratch diff was 5 lines in and 26 out. It was not
qualified across exception, preview, V30 or debug paths, and would make reset
semantics depend on future structure field placement.

Configured-CMOS baseline/candidate/candidate/baseline fixed-work times were
19377/20807/19279/18549 ms on x64 and 21062/24505/21268/22447 ms on x86;
all reached 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`.
Both x64 comparisons favored the original; x86 changed direction and displayed
substantial host variation. Although the candidate reduces source lines,
the timings do not demonstrate an improvement and its implicit layout
coupling weakens maintainability. It was rejected without modifying production.
The fixed checkpoint does not prove complete state equivalence or gameplay
performance. Log SHA-256:
`E1E5F6249C671A3387C0AE331918612AE221A74D1BE054E678A414F53EF677BB`.

## 486 access-penalty zero case

A configured-CMOS, task-only counter checked whether every input relevant to
the 486 translation/alignment/descriptor access-penalty calculation was zero.
It observed 9,293,112 zero cases in 10,003,020 calls on x64 and 9,295,787 in
10,006,312 calls on x86, approximately 93% at the startup checkpoint. The
counter is diagnostic and does not characterize late gameplay or nonzero/error
cases.

A scratch candidate returned the same zero ticks and unmodeled result when all
those inputs were zero, retaining the original validation and computation for
every other case. Its fixed-work baseline/candidate/candidate/baseline times
were 23589/21068/20400/19628 ms on x64 and 27041/24141/24834/26541 ms on x86;
each reached 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`.
The x86 pairs favor the candidate, while x64 changes direction. This does not
establish a stable dual-width or gameplay gain or complete-state equivalence.
The prototype repeats a long list of fields ahead of the existing validation,
adding a separate fast-case condition and making maintenance of the error
contract more fragile. It was not subjected to semantic qualification or
merged. Counts and comparison log SHA-256 respectively:
`584C1E60D69FDC49FCB91CED218F07EBB8BD6831BF5359E2A204A8211C24D609` /
`05A2EC0525BCFEF628D1CF4A6D1DCAA84509263F62BDB64E33BCE76F3B748A2A`.

A second scratch candidate retained the existing validation first and then
returned early when its validated inputs implied zero access penalty. It
avoided duplicating the full validation, but introduced another set of
conditions and repeated part of the result classification. The configured
baseline/candidate/candidate/baseline times were 22780/22633/22292/19532 ms
on x64 and 25663/24639/26000/26494 ms on x86, with the same fixed checkpoint.
The x64 comparisons again changed direction and the small x86 differences do
not establish an interactive gain. This version was not semantically
qualified or merged; matching the checkpoint is not proof of equivalence.
Log SHA-256:
`8F5567BB54A9BAB9CA39800A35BE41D07F291C8C037F75D6026EEC4E7FF5F115`.

The [CMOS probe correction](t582-s34-cmos-probe-correction.md) separately
records why S31/S32 fixed-work results used the empty-CMOS path and supplies a
new configured-seed comparison of S33. No production code or public contract
changed in this audit.
