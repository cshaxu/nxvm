# Td S138: Controller Phase Program Decomposition

`M5:TD:S138:CONTROLLER-PROGRAM-DECOMPOSITION:OK`

The former aggregate controller/device candidate is retained only as shared
context. Queue candidates now map one finite controller or inseparable
controller-plus-media chain to its source-admission, dependency and stop
boundary: PIC, DMA, PIT, RTC/CMOS, KBC/NMI, FDC plus logical media, VADP, and
source-blocked HDC/ATA.

PIC precedes all shared IRQ consumers. DMA follows PIC and precedes FDC. PIT,
RTC and KBC consume PIC independently. FDC consumes both PIC and DMA. VADP
consumes the T449 transaction boundary without a display-owned time path. HDC
remains after the shared interrupt boundary and cannot be admitted before ATA
source and selected backing/personality evidence. The existing integration
candidate follows the accepted contracts rather than recreating them.

This changes no runtime code, ABI, source admission or profile behavior.
