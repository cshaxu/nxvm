# M5 T480 VADP IBM VGA L3 Capability

T480 admits the queued IBM VGA controller capability without claiming that an
existing IBM 5170 or DeskPro profile shipped with VGA. It retains VADP as the
only owner of guest video state, ports, mapped memory and copied snapshots.

| Subtask | Accepted result |
| --- | --- |
| S1 | Accepted: the finite IBM primary/cross-model ledger classifies controller state, attribute access, chain-4, aperture, latches, DAC, Mode 13h, CRTC mapping, timing and profile binding. It records PCjs/QEMU chain-4 limits and leaves every current profile unselected. |
