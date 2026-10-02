# M5 T540 S11 FDC Port Routes

FDC now publishes status, data, optional DIR and diagnostic reads, and DOR,
data and optional control writes through one Core-owned typed route batch.
Core alone validates and publishes that batch. The 8272A chip still owns its
command, result, DRQ and IRQ behavior; the NXVM board attachment still owns
media change sampling, drive mechanics, DMA and PIC wiring. Neither chip
behavior nor timing inputs changed.

The read provider replaces only the low byte of the current port value. Its
data-register path begins with that byte so a chip read which supplies no new
byte retains the former port-latch result. DIR samples external media change
only when the guest reads it. The write provider passes the same low byte to
the existing DOR, data and rate-control operations. If route publication
fails, it destroys the just-created chip; the board constructor then clears
the attachment and topology. Core's batch leaves pre-existing routes intact
and publishes no partial FDC route.

## Scope and actual-diff review

- Production changes are confined to `devices/{fdc.c,fdc.h,machine_board.c}`:
  84 added, 98 removed lines (net -14). Five owner-local tests add 8 and
  remove 7 lines (net +1). The source-boundary verifier adds 23 lines. No
  Shared, MyNES, profile, firmware, INI or media input changed.
- The FDC attachment now stores the opaque Core owner rather than `t_port *`.
  Its raw callbacks and individual `core_machine_port_add_*` calls are gone.
  The board constructor no longer repeats a port-availability check or holds
  a registration checkpoint; Core's atomic batch owns conflict and rollback.
  Test-only port inspection remains a white-box fixture, not a production
  route or second state owner.
- The existing port-assembly fixture injects FDC chip-create and each route
  allocation failure, checks no partial FDC route or pending DMA request,
  then retries on the same machine. Controller and media-change tests retain
  the previous status/data/DOR/DIR and DRQ/IRQ coverage. The strengthened
  static verifier rejects a revived raw FDC port path.
- A full adapter sweep finds no raw port-table reference in `fdc.c` or
  `fdc.h`. VADP, HDC, RTC and remaining board ports are assigned to S12;
  KBC A20, DMA/refresh and FDC memory/signal exchanges belong to the later
  bounded Core/board cut. Generic Core port storage and CPU bus remain for
  S17. S11 does not claim that physical move or board extraction complete.

## Verification

- Both x64 and x86 unit builds compile. The focused FDC/port selection passed
  16/16 per width. The complete repository-only unit suites passed 467/467
  per width (x64 at `-j 4`, x86 clean rerun at `-j 4`). An earlier x86
  parallel run encountered a shared Win32 modal-test timing failure; that
  case passed in isolation and the complete lower-concurrency rerun passed.
- The registered FDC state-machine and T345 ownership verifiers pass on both
  widths. NXVM documentation governance and `git diff --check` pass.
- All four optimized Release pairs were rebuilt in `assets/nxvm/<profile>/`.
  Every x64 file is `pei-x86-64`, every x86 file is `pei-i386`, and none has
  a `.debug` section. Adjacent INIs and MyNES files remain untouched.

| Profile | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| XT 5160 | `AC04CCEA1E6DE6AFEE31F8235B819DEDCF9E11353567C9560450D2EFAD9E2909` | `61B46A2100D939ED602D77A8CDEEE40CBA319D36504B054DC3544F58168AC89C` |
| AT 5170 | `824BC01198219A324807AB9CD9C4C89E3FD362489E938530BECBF57DF7EA7C64` | `DBBED7958C1CF5983FE32F0EFEEE809BB1B67B261B35351721B549ACB544EC10` |
| Model 40 | `3C406286E17EF9279F84D92901B580BEC592019E7CE18747C51CEC376382C4D8` | `7168800A046C3DD884020D143F8C4FCAD5F301802F3C66D29AB31284184658E3` |
| Default PC/AT | `22E623C52ED7576C5BBD0776C318451C2D7D66823CBD59FE238DE19639AC0F48` | `EAFDD2BF5CBCCB4EECDFF7493FEB6D52E43A8D8E9B50C1B02C9100D0BFFF2915` |

No new L3 or other timing classification is claimed. The full external
four-profile integration gate remains due at T540 closure.
