# T582 S26 — CPU parity calculation candidate

The CPU parity flag routine counts set bits in the low result byte with a loop. A scratch-only candidate instead XOR-folded the byte and inverted its low bit, without changing flags ownership, timing formulas, state, or interfaces. An independent mathematical check covered all 256 byte values; this was not a C-path, CPU-profile, exception or Debug qualification.

Both source variants were compiled directly with the same x86/x64 Release options and linked to the current PC110 libraries and private media. Every run reached 10,000,000 retired instructions, 79,964,294 guest ticks and PC `0003dbc5`:

| Width | Baseline then candidate, ms | Candidate then baseline, ms |
| --- | --- | --- |
| x64 | 20,835 / 19,369 | 19,272 / 18,680 |
| x86 | 25,012 / 23,063 | 24,052 / 24,279 |

The first pair suggested improvement on both widths, but the reversed x64 pair changed direction and the reversed x86 difference was small. Sequential runs are affected by host variation; the checkpoint does not establish full machine-state equivalence. No stable cross-width gain is established, so the candidate was not introduced into production and no full receiving suite or EXE rebuild was justified. This startup checkpoint is not the actual Game workload and does not establish playable performance.

Scratch logs SHA-256 (forward, reverse): `7130D17DC70003939E60192024CD6C68B3B102A86809ED5C42F761F15B813134`, `77A6F05216B1605DA51BCD2A5868E7B29729A85BE3DB35A03F85530CFD7A10A6`.
