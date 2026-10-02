# M5 T540 S47 Board-Owned DMA Controllers

## Scope and actual diff

The DMA latch and primary/secondary controller instances now reside in the
existing sole `core_machine_board_state`, rather than flat `core_machine`.
No mirror, accessor API or second controller path was added. The same board
constructor initializes both controllers and the latch in the same order;
the same request/acknowledge, PIT1 refresh, FDC/HDC DRQ, transaction advance,
deadline, reset and finalization calls use their new address. The DMA clock
plan and timing formulas are unchanged.

Three production and thirteen direct test consumers were retargeted. The
DMA arbitration source gate additionally rejects the board DMA instance
paths in the neutral scheduler. Tracked source/gate/test diff adds **139**
and removes **129** lines (net +10), consisting of field-path migration,
eight needed private-header includes and the gate terms. No DMA algorithm,
profile, INI, firmware, media, Shared or MyNES source was changed.

## Verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: x64 and x86 pass, including the
  DMA arbitration boundary, strict compilation and document governance.
- Fixed-profile external boot probes, **once per profile and width**:
  Default reaches `dos-prompt`; XT, IBM 5170 and Model 40 reach
  `installer-running`. **8/8** accepted terminals.
- Eight optimized 0540 EXEs rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. PE format is correct for four x64
  and four x86 products; `objdump -h` finds zero `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `04FAC6B2FD1AC51541C0FFD4F037BCC06F27AE33E9D60F658AE3018350744444`, `6C632091DC0BCE8C2CA428B397412B0BDB4D571EA19E70FE9483046D48B790AB`.
  - Default: `9DB62710F4FD86E14D9301190076F5AB34BFAB61B4B97983C5A8C0977BBBA6CA`, `BD70E3507CF0F8A24910FE0C1BE5D773382E777A798897E1F4914480DBB96AF1`.
  - XT: `39B8F31E0F75A295F55C109564B40E75078EE861EB246BBC629721C1BD32F86C`, `72DE88791A0132FD5E81FE2C1B84765B7C849370C563E537F972DE8518B4F3AE`.
  - IBM 5170: `F3640BEB935CEA1DCB2C285EC6212E37C082D13E00AFAE0A35F8304D6BF6C6B6`, `AC5761ADC6C1B84DAAA46936D7169F67D02495E62278AF8CEF2A86D9A89B006A`.

S47 closes only the DMA group. S48 receives RTC; T540 remains open.
