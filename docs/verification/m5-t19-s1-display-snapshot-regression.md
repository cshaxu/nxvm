# M5 T19 S1 Display Snapshot Regression Repair

The full-PC FDD regression was bisected with the frozen local fixture
`fdd.img` (`FADEB3A27C6A0E1CF582DDE0B9AECB7E5D30678F2F967F2F4562F167CC0CB1D5`).
`5878aa5` and `3536f51` reached the DOS 6.22 prompt; `ff4cd24` and current
T18 were markedly slow and stopped after `HIMEM.SYS`. The bisection isolates
`ff4cd24` as the first bad commit.

That commit captured every character and attribute before checking whether the
display or cursor changed. The default 80x25 text provider therefore performed
the full copy on each guest refresh. The repair reads snapshot metadata first
and returns before the text-buffer copy when both change flags are clear.

The isolated current-source build containing only this repair returned to the
DOS prompt under the same FDD startup procedure. No raw recorder trace was
created. The committed source rebuilt with GCC; the dependency DAG and core
contract gates passed, and the retained Console reached `Console>` then exited
with status 0.

The task artifact is the ignored `build/output/nxvm-m5_t19.exe`:
SHA-256 `1D55A19F8F2AB78E909A093FF9B77BB4196D12CCB2C67A29BF2E115942743F43`.
It emits `Neko's x86 Virtual Machine [0.4.015d.m5t19]` and is a local
developer artifact, not release evidence.
