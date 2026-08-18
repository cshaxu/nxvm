# T406 S1 KBC Serial Cadence Evidence

`M5:T406:S1:KBC-SERIAL-CADENCE:OK`

T406 adds a `generic-at` native-keyboard serial delivery cadence to the existing Core KBC serial queue. The Model-40 private composition selects one virtual tick; this is not an original Compaq measurement or physical/L3 claim. PCjs/86Box influenced only the separation of keyboard delivery from board logic; no third-party source, firmware, media or configuration was imported.

The Core KBC remains the sole input/FIFO/IRQ owner. A nonzero cadence releases one queued native byte per expiry; zero preserves immediate delivery. Reset clears queued delivery state while retaining configured timing.

Verification: focused `core-machine-kbc-serial-cadence-smoke` passes; full `current-gate` passes 287/287; documentation governance passes. Developer artifact: `vm-0-5-0406`, SHA-256 `81357370B706709088DAB0CD5F5AE42AA15006A9DD95F665DCEBE486D9B35C4E`.

Remaining transfer: exact keyboard serial rate, 8042 microcode/FIFO and IRQ latency, board phase timing, and physical/L3 acceptance remain unproven.