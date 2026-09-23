# T393 S4 FDC READ DATA C0A Diagnostic

The existing Model-40 BYOB retirement capture now has an opt-in
`--fdc-read-data-diagnostic` mode. It retains no firmware, media, raw trace,
PC, instruction bytes, or mutable device/session pointer.

The diagnostic establishes C0A only from the first observed post-C0 immediate
`IN AL, 61h` retirement. At that point it snapshots the already copied
Model-40 FDC terminal-result sequence. It reports success only when a later
copied result has a strictly greater sequence and is a successful drive-0
`READ DATA` command (`E6h`). The one-retirement run budget makes this a
conservative ordering boundary: a result whose relation to the C0A retirement
cannot be separated is not accepted.

The synthetic regression rejects the baseline sequence, another drive,
a failed result, and reset-invalidated state; it accepts only the later
successful drive-0 result. Focused Core FDC, Model-40 FDC, and capture smokes
pass. This is bounded candidate observation only: it neither proves C1 nor
physical board/timing/L3 qualification.
One owner-managed BYOB execution used the existing DeskPro ROM pair and a
5.25-inch MS-DOS boot floppy solely as external inputs. It reached the fixed
2,000,000-retirement containment limit with C0A reached, 2,000,000 classified
retirements and zero unallocated retirements, but did not observe a later
successful drive-0 `READ DATA` result. The outcome is therefore a bounded
non-success; it does not select a C1, board, physical-time, or L3 receiver.
No asset was changed, copied into the repository, catalogued, or committed.
