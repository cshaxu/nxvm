# M3 T3 S1 Full-PC Profile Verification

## Result

- `nxvm.full_pc` now has an explicit runtime descriptor containing CPU, RAM,
  ports, PIC, PIT, keyboard/text display, BIOS, CMOS, DMA, FDC/FDD, HDC/HDD,
  and VADP. It is the only current profile that permits disk boot.
- The retained NXVM device graph is available through a single-session adapter,
  not through the core Machine. Its FDD/HDD insertion, boot selection, reset,
  stop, and teardown are explicit.
- Owner-provided `fdd.img` and `hdd.img` matched the identities in
  `docs/fixtures/m1-local-images.md` before the run.
- `nxvm-full-pc-profile-smoke` printed `M3:T3:S1:FULL-PC-PROFILE:OK` after
  separately configuring FDD and HDD boot. Both configurations exposed the
  baseline reset vector `F000:FFF0`.
- The M1 `nxvm-baseline` target remained buildable; imported source has no
  diff. The prior M1 ten-second FDD/HDD watchdog traces and DOS `INT 21h`
  checkpoints therefore remain the behavioral regression record for the
  unchanged legacy adapter path.
- No guest media, trace output, or product artifact was committed or created.

## Boundary State

M3 T3 S1 makes whole-PC composition explicit without claiming that BIOS/device
behavior has already migrated into `core`. M4 assigns firmware ownership; M5
performs the product-facing implementation migration.
