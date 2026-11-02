# T582 S40 — stage-closure review

The owner directed stage closure even if further performance work is unlikely
to pay off. This is not acceptance of the original PC110 usability target.
The final S39 private-media samples reached the menu after 183 seconds on x64
and 251 seconds on x86; response appeared after a further 8 and 11 seconds,
respectively. The x86 response capture was incomplete. These are individual
samples rather than a controlled before/after comparison. Native desktop and
RDP responsiveness were not verified by these probes.

T582 retained bounded changes to existing CPU execution, timing, clock,
memory-route, PIC, PIT, DMA/FDC and F65535 paths after their applicable
regressions and receiver gates. The complete individual proposals, rejected
experiments and actual measurements remain in the T582 evidence. The final
Shared clock delivery is 4fb901430; its NXVM and MyRaidenII receivers are
340baabaf and fd4ecfedd. Local reductions do not prove usable boot speed.

The product review covered the five PC App source trees and, as a limited
cross-product check, the MyNES and MyRaidenII App trees. It inspected their
component ownership, construction/configuration, representative failure and
teardown paths, and scanned C/H for obsolete work markers, disabled blocks and
several unsafe CRT calls. It is not a line-by-line proof that every App is
defect-free. MyNES and MyRaidenII have independent product tasks and were not
modified here.

The review confirmed that MyPC110 accepted a requested 4 MiB configuration
while always constructing 20 MiB. S40 rejects unsupported explicit memory
sizes before allocation, while preserving the omitted and 20 MiB cases; an
existing construction test covers rejection and product composition checks the
20 MiB case. Additional supported PC110 memory models remain a separate debt.

The x64 cached build and full unit run passed 712/712; the MyPC110
construction test and two direct product/firmware integration tests passed.
The x86 cached build, the same three focused tests, and its full unit run
also passed 712/712. The previously delivered S39 receiver, rather than this
S40 run, supplied the last full dual-width integration results.

The deployed MyPC110 x64 executable SHA-256 is
`11F05D0087E0F0CE4F90C7A9217294801298355B285D13E718E497A93B306D60`;
the x86 executable SHA-256 is
`63876FE16FA10B1DDD0349DBF490B031C79EFA7ADBDC39F12B64DB6153A42875`.
Both match their respective cached build output; the build's PE architecture
checks passed. Other product executables were unchanged in S40.

The pre-existing X86 Core masked multi-byte boundary and provider-decline
fallback defect remains unresolved; its independent reproductions and limits
are recorded in the [S39 evidence](t582-s39-execution-convergence.md) and the
NXVM debt ledger. No Shared source was changed to hide this issue. The user's
pre-existing queue edits are preserved separately from this work.
