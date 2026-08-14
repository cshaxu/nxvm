# T366 S7: Model 339 Firmware And FDC Topology

The selected profile declares an abstract IBM 5170 Rev.3 firmware slot and a
drive-A field upgrade. It retains the existing CMOS floppy byte `40h` (drive A
type 4), FDC ports `3F2h`, `3F4h`, `3F5h`, `3F7h`, IRQ6 and DMA channel 2.
The firmware slot contains no vendor bytes, path or hash. The TEAC FD-235HF-A529
is a later field upgrade, not a factory Model 339 claim; no hard disk is added.

The profile choice is supported by the [IBM 319/339 announcement](https://sharktastica.co.uk/resources/docs/IBM_186-052_5170-319-339_86.pdf),
the [PCjs Rev.3 chronology](https://www.pcjs.org/machines/pcx86/ibm/5170/rom/),
and the [TEAC specification](https://ftpmirror.your.org/pub/misc/bitsavers/pdf/teac/FD-235HFA5XX_Specification_Rev_B.pdf).
Those materials establish identity/capability, not controller-service timing.

`vm-ibm-5170-model-339-firmware-fdc-topology-smoke` proves the declared slot,
field-upgrade flag, CMOS type, FDC port surface, IRQ6 and DMA2 composition. It
emits `M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK`.

IBM ROM behavior, 765-family command timing, MFM/ST-506, ATA/HDC, FDC service
timing and bus/cycle timing remain later transfers.
