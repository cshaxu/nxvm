# M5 T503 Controller Signal-Chain Convergence

T503 closes the selected controller-to-consumer routes through one Core owner
per device: PIC, PIT, DMA, 8272A FDC, fixed-disk personalities, RTC/CMOS,
8042/XT keyboard and VADP. S3--S10 independently reconciled source, code and
available external models, repairing only owner-local route defects.

S11 replays the finite route ledger across IBM 5160, IBM 5170 Model 339,
DeskPro 386 Model-40 and default-at. The focused cohort passes 27/27, complete
unit passes 312/312, and BYOB integration passes 20/20. The only closure-time
compile defect was a VADP-private declaration order issue; it was fixed without
adding state or a new path.

The current stripped Release artifact is `build/output/nxvm_0_5_0503.exe`,
1,240,043 bytes, SHA-256
`3AF1408F5E1C3760CF2A3D38D0EA8B7A739122BD2F4B7FBB49A527566E7A8373`.
Physical timing and unselected capabilities remain at their existing named
TODO receivers; no selected T503 causal route remains open.
