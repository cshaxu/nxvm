# T582 S26 — ModR/M register reread

The ModR/M decoder reads a register operand into `crm`; `_m_read_rm` later
clears that temporary and reads the register again. Memory operands instead
perform the actual logical read at the latter point. Widths are not always
identical: some callers decode a 32-bit operand and then read only 16 bits.
The earlier Game observation counted approximately 3.41 million register
operands through the relevant path in its drawing interval; stage boundaries
and counts are approximate, not per-instruction performance attribution.

A task-only instrumented implementation preserved both reads and compared the
cached temporary, masked to the requested width, against the fresh value.
The private-media startup checkpoint exercised 1,049,052 register rereads
with zero differences. Independently relinked existing CPU tests passed
122/122 on each host width; each suite exercised 2,030,137 register rereads
with zero observed differences. Instrumentation changes execution cost; these
tests and the startup workload do not establish equivalence for every
possible callback, CPU context or guest instruction sequence.

A second task-only candidate reused the decoded temporary for 1-, 2- and
4-byte register reads, preserving narrowing, memory reads and the fallback
for other widths. At the 10,000,000-instruction private-media checkpoint,
baseline/candidate/candidate/baseline times were
22,956/22,273/23,778/20,866 ms on x64 and
24,029/24,711/23,923/23,052 ms on x86. Every run reported 79,964,294 guest
ticks and PC `0003dbc5`, not full-state equivalence. The candidate adds
branches and lines and does not show a reliable gain; x86 is slower in both
ordered pairs. It was not adopted, and no Game or full receiver test was
claimed for it. No production code, test package, owner media or EXE changed.
