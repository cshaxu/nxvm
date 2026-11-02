# M5 T582 — MyPC110 startup performance: owner-directed stage closure

The owner closed T582 after its delivered performance work and final quality
review, accepting that further work is deferred. This is not a claim that
PC110 startup or interactive usability reached the original target.

The task optimized existing CPU execution and timing, clock and device event
handling, memory/port routing and F65535 presentation paths. Some local
improvements were measured; mixed or unproven candidates were not retained.
The final bounded Shared clock change is `4fb901430`, with NXVM and MyRaidenII
receivers `340baabaf` and `fd4ecfedd`. Its full dual-width qualification and
fourteen artifact record are in the S39 evidence.

S40 audited the product-owned paths and fixed MyPC110 silently ignoring an
unsupported RAM request: `bb712dc88` rejects an unsupported explicit size,
retains 20 MiB and the default, and publishes the two updated MyPC110 EXEs.
The cached unit suites passed 712/712 per architecture; the affected PC110
construction and two focused integration tests passed per architecture.
Other products and Shared production code were unchanged in S40.

The latest private-media samples still needed minutes to reach the sampled
interface. Real desktop/RDP usability is unverified. The separately reproduced
Shared masked-span/provider-decline memory-routing defect remains unresolved.
These items and additional PC110 RAM models are retained in the NXVM debt
ledger. Details, limitations, artifacts and links to the complete S39 evidence
are in [S40 stage-closure evidence](../etc/evidence/t582-s40-stage-closure.md).
