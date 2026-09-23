# T366 S5: Model 339 Session Composition

`vm_session_config.profile_kind` now selects either the retained
`default-pc-at` descriptor or `ibm-5170-model-339`. The selected Model 339
descriptor fixes the existing session composition to an 80286 CPU, 512 KB
initial RAM, 512 KB system-board parity binding at port `61h`, a 1.44 MB
floppy CMOS type, and no fixed-disk topology. Its fixed-disk CMOS fields are
zero; it has no active ATA port leaves or ATA controller.

The selection reaches the existing session lifecycle rather than a second
machine path. It suppresses HDC media initialization/binding, controller
configuration, firmware registration, refresh/reset/finalize, console HDD
operations, status output, hard-disk boot preference and configuration-time
HDD input. The current default session retains its 80386/ATA descriptor and
regression behavior.

`vm-ibm-5170-model-339-composition-smoke` emits
`M5:T366:S5:MODEL339-COMPOSITION:OK`. It proves both profile descriptors,
Model 339 CPU/RAM/parity state, absent ATA port `1F0h`, rejected session HDD
input and creation, and retained default ATA ownership. Existing default
profile, PC/AT composition and ATA-port proofs also pass.

This is a bounded composition contract, not an exact firmware, display or
clock claim. The repository-authored firmware remains distinct from the IBM
Rev.3 ROM slots. Current VADP configuration still includes its retained
EGA-derived surface and therefore cannot establish the selected IBM CGA
contract; a later T366 device/profile S owns that separation. Exact 8 MHz
guest-time, FDC service/DRQ/DMA timing, MFM/ST-506, I/O-channel check and all
bus availability/physical timing remain transfers.
