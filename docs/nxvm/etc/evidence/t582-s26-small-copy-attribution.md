# T582 S26 — small memory-copy attribution

A scratch link wrapper attributed small `memcpy` calls in a private-media x64
PC110 startup run. At 10,000,000 instructions it recorded 15,884,106 calls
from `mypc110_firmware_read` and 5,290,231 from ordinary RAM writes. The
firmware reads included 5,784,432 two-byte and 5,260,219 thirteen-byte calls;
ordinary RAM writes included 5,213,501 four-byte calls. Other sizes and call
sites were also present. Caller addresses were mapped to the linked executable
and inspected in disassembly. The wrapper adds atomic counting and only sees
calls reaching the linked `memcpy` symbol, so these counts do not establish
exclusive cost or Game-stage frequency.

A task-only candidate selected a fixed-size `lib_memory_copy` when an ordinary
RAM write was exactly four bytes and retained the existing arbitrary-size copy
otherwise. Routing, snoop and parity processing were unchanged. In ordered
baseline/candidate/candidate/baseline fixed-work runs, x64 times were
21,122/21,966/19,322/18,946 ms; x86 times were
23,427/22,839/22,499/22,817 ms. Each run reported 10,000,000 instructions,
79,822,202 ticks and PC `00039119`. The measured range varies considerably;
x64 does not support a benefit, while x86 shows only a small difference. This
checkpoint is not complete-state equivalence, a real Game comparison, or
native Window/RDP validation.

The candidate adds a runtime branch and does not reduce the asymptotic work or
production source size. With no reliable cross-width gain it was not adopted.
No Shared source, public interface, test, product executable or owner media
changed in this experiment. S26 remains open for a substantial PC110
throughput improvement.
