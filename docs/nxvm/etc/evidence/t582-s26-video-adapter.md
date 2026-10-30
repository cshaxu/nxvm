# T582 S26 — PC110 display aperture validation

The PC110 Machine memory router selects the display adapter through its
existing aperture query. The adapter's read and write callbacks then repeated
that query, while the F65535 chip's actual read/write each call the same
window-offset validator themselves. The adapter now leaves the final window
check to the chip. It still returns `UNSUPPORTED` for a null owner, null data
address or zero length, matching its previous callback behavior; for non-null,
nonzero inputs the chip uses the same pure window validator and returns its
original status. Observe-only reads are passed through unchanged. No new state,
interface, provider or production path is introduced; this is an App-only
change of +2/-4 lines.

The earlier Game-stage probe observed 78,841 reads and 266,911 writes in an
approximate drawing interval; this change eliminates the adapter's one extra
query per accepted access. Counts include probe overhead and do not establish
an elapsed-time improvement. The route lookup and the chip's real access
validation remain necessary.

The existing PC110 route tests exercise real Machine-to-chip reads, writes,
window changes and ownership. The existing F65535 tests cover window bounds,
invalid spans, pixel behavior and observation. On x86 and x64, all ten PC110
unit tests and the chip contract test passed, and all nine PC110 integration
tests passed. Both PC110 executables were rebuilt and deployed. No Shared
source/test or other App was changed by this batch.

A private-media x64 product-path probe reached a visually verified complete
Personaware Game scoreboard 29 one-second samples after Enter and exited after
266.64 seconds. Its saved frame SHA-256 is
`C5B5C336A477F39751CC3BB643ECF5CD2FDF0177BDCAA510A506B2D2D44942D9`.
The previous normal run reached a complete frame after 25 samples, but these
sequential runs did not start from proven identical complete guest and host
states. They neither prove a regression nor a performance improvement. Input
was injected through Machine, not native Window/RDP.

The final artifacts' PE widths and hashes are recorded in
[the inventory](t582-s26-video-adapter-artifacts.json). This bounded cleanup
does not itself establish playable speed or native Window/RDP behavior. T582
S26 remains active.
