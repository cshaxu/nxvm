# M5 T35 S1 FDC Authority

`vfdc` now directly aliases the one `t_fdc` embedded in
`vm_composition_live_machine`. Its port registration, DMA/IRQ callbacks, FDD
links, reset, and finalization order are unchanged.

Windows GCC, `M5:T35:S1:FDC-AUTHORITY:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t35.exe`.
SHA-256: `F2A4C5531619E2CACC2386CA8EF1D956DD31944CE14FFBF641673A8FB1F86537`.
