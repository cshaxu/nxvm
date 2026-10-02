# M5 T540 S48 Board-Owned RTC

## Scope and actual diff

The RTC chip pointer and CMOS selected-register latch now reside in the
existing sole `core_machine_board_state`, rather than flat `core_machine`.
There is no copied RTC phase, mirror, accessor API or second chip path. The
same board configuration creates the RTC, installs the same port routes and
default seed, rolls back failed port installation, advances/reset/destroys
the chip and observes its IRQ deadline in the same order. RTC formulas,
interrupt semantics and CMOS contents are unchanged.

Three production and ten direct test consumers were retargeted. The RTC
boundary gate now also rejects direct RTC chip or selected-register access
from the neutral scheduler. A multi-session test previously compared the
addresses of two RTC pointer *slots*, which could not detect a shared chip;
it now compares the actual RTC instance pointers. Tracked source/gate/test
diff adds **70** and removes **55** lines (net +15), chiefly field paths,
four private-header includes and the gate. No profile, INI, firmware, media,
Shared or MyNES source was changed.

## Verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: x64 and x86 pass, including the RTC
  boundary, strict compilation and document governance.
- Fixed-profile external boot probes, **once per profile and width**:
  Default reaches `dos-prompt`; XT, IBM 5170 and Model 40 reach
  `installer-running`. **8/8** accepted terminals.
- Eight optimized 0540 EXEs rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. PE format is correct for four x64
  and four x86 products; `objdump -h` finds zero `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `3F5224964E1D4891D9462F80B78D6711B209AEB078FB8F4EC5FB02CB0FCC6BE2`, `E3949593222F4A90445A05EF0118D28CAA60779EB06B06E99975CB335ECE2850`.
  - Default: `2AD21231C5BCC46DDB51E3037DCDDC8017E91F98C939E38F6ABC80539638BE45`, `A132DEA2CF938C0B283D2D505DC46FE34DD7687E8FDC53D0AE4F925925B05988`.
  - XT: `0E4FB13079135BCAC480F16A31F743028E6C951CD5949898642324286F51DDA3`, `0EC2FA82E3A8A2CC09C08D31E78FF41EF3AD59BC137A5AD8AF76D7CC28B58B41`.
  - IBM 5170: `ABC91390C2DBEDE862F5F99CC446282BEE1DAB26C0A73C79FA78468B95A0BE55`, `C990BB2CFACFB91A38DEE6213D5E38B3277841BDAAE201391E1E4D1CCFB14950`.

S48 closes only the RTC group. S49 receives FDC; T540 remains open.
