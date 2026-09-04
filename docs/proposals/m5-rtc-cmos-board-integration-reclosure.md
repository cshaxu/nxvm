# M5 RTC-CMOS Manual And Board-Integration Reclosure

Re-audit MC146818 register semantics, update/alarm/periodic interrupts,
divider/reset behaviour, NMI masking and the standard 64-byte CMOS boundary.
Prove each profile's immutable seed to Core-owned writable board configuration,
RTC event delivery through PIC, and BIOS-visible checksum/configuration path.
Do not create a second calendar, YAML runtime mutation or undocumented NVRAM
extension.  Apply the shared
[controller reclosure program](m5-controller-board-integration-reclosure-program.md).

Subtasks: List-1/List-2 reconciliation; owner/seed/event repair;
cross-profile ROM and complete-gate closure.
