# Queue

M0 is owner-accepted and closed. [M1 T1](../history/M1-T1-board-and-source-freeze.md)
closed the source/board/set and CPU integration-contract freeze. The V30
receiver is now [M1 T2](../history/M1-T2-v30.md). The remaining
candidates below are unadmitted; numeric T identifiers are allocated only on
admission in MyRaidenII's independent sequence, not NXVM's sequence.

1. [Implement board bus, interrupts and Seibu COP](../proposals/m1-board-cop.md)
2. [Implement and qualify video](../proposals/m1-video.md)
3. [Implement and qualify sound](../proposals/m1-sound.md)
4. [Connect Machine, Product, configuration and controls](../proposals/m1-product.md)
5. [Qualify complete gameplay and release](../proposals/m1-qualification.md)

These are receiving boundaries, not already admitted implementations.
Each admission freezes bounded S batches and a finite verification ledger.
Minimal headless and presentation probes belong to their owning earlier task;
the Product task completes the real host route, not a second execution path.
Reorder or merge batches only with recorded evidence and owner agreement.
