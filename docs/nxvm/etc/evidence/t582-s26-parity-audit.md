# T582 S26 — CPU parity calculation candidate

The CPU parity flag routine counts set bits in the low result byte with a loop. A scratch-only candidate instead XOR-folded the byte and inverted its low bit, without changing flags ownership, timing formulas, state, or interfaces. An independent mathematical check covered all 256 byte values; this was not a C-path, CPU-profile, exception or Debug qualification.

Both source variants were compiled directly with the same x86/x64 Release options and linked to the current PC110 libraries and private media. Every run reached 10,000,000 retired instructions, 79,964,294 guest ticks and PC `0003dbc5`:

| Width | Baseline then candidate, ms | Candidate then baseline, ms |
| --- | --- | --- |
| x64 | 20,835 / 19,369 | 19,272 / 18,680 |
| x86 | 25,012 / 23,063 | 24,052 / 24,279 |

The first pair suggested improvement on both widths, but the reversed x64 pair changed direction and the reversed x86 difference was small. Sequential runs are affected by host variation; the checkpoint does not establish full machine-state equivalence. No stable cross-width gain is established, so the candidate was not introduced into production and no full receiving suite or EXE rebuild was justified. This startup checkpoint is not the actual Game workload and does not establish playable performance.

Scratch logs SHA-256 (forward, reverse): `7130D17DC70003939E60192024CD6C68B3B102A86809ED5C42F761F15B813134`, `77A6F05216B1605DA51BCD2A5868E7B29729A85BE3DB35A03F85530CFD7A10A6`.

## Owner-approved simplification

The owner subsequently directed that a net production-code reduction is worth
adopting without a large measured gain when behavior and other properties do
not clearly worsen. The parity routine now uses three fixed XOR folds and the
low bit, reducing the production diff by three net lines. Since the input is
always eight bits, both algorithms are O(1); the change removes the
data-dependent loop but does not change asymptotic complexity. The earlier
fixed-work timings remain inconclusive and do not establish a speedup.

The existing CPU legacy-ALU test was extended to execute `TEST AL, AL` for all
256 values and all seven supported CPU profiles, starting each case with the
opposite parity bit and checking EAX, instruction advance and preserved IF.
It passed on both host architectures. This exercises the real production
flag path; the additional test is 31 net lines in an existing test, with no
new public interface or test-only production path. The fixed guest timing
formula is unchanged.

Both cached host architectures passed the full 712-test unit suite. Default
integration covered all 48 cases on each architecture, including desktop and
PC110 firmware cases. On each width the initial aggregate had two failures:
the session INI and default boot-matrix test executables still contained the
previously selected DeskPro binding. Relinking those two targets for the
default profile and rerunning them passed 2/2 on each width. Thus every case
passed in its correct configuration, but the initial aggregates did not pass
in a single invocation. Subsequent profile switching explicitly rebuilt the
selected test targets and, on restoring default, rebuilt these targets too.

The selected My5160, My5170 and DeskPro integration suites passed 3/3, 5/5
and 6/6 on each width. The external selected-set MyRaidenII startup and
deployed Console probes passed for both widths. Fourteen current product
executables were built or confirmed up to date; their PE widths and hashes are
recorded in [the artifact inventory](t582-s26-parity-artifacts.json). MyNES
does not link X86 Core, and its two executables were unchanged. X86 Core source
and test manifests and the NXVM documentation gate passed. These checks do not
establish interactive PC110 RDP gameplay or real-time performance.
