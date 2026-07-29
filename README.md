# ntvdm64

`ntvdm64` is a native C compatibility runtime for running selected DOS
programs on 64-bit Windows. It owns a DOS-oriented virtual machine and host
bridge. It may dispatch Win16 NE programs to a separately installed WineVDM
backend; WineVDM is not embedded in this repository.

The initial product target is explicit launching:

```text
ntvdm64 run PROGRAM.COM [arguments]
ntvdm64 run PROGRAM.EXE [arguments]
```

Explorer integration is a later, opt-in delivery feature. It must not depend
on global loader injection, undocumented Windows patching, or replacing the
normal PE loader.

## Delivery Model

1. **M0 governance and laboratory**: establish legal, evidence, executable
   format, and compatibility-test boundaries.
2. **M1 DOS launch spine**: load a COM program into a synthetic DOS process
   environment and support controlled termination and text output.
3. **M2 DOS host bridge**: add MZ loading, DOS file/console/input services,
   command-line execution, and a repeatable compatibility corpus.
4. **M3 interactive DOS compatibility**: add mouse, graphics, timer, and
   selected XMS/EMS behavior through explicit profiles.
5. **M4 protected-mode and Win16 routing**: add only evidence-backed DPMI
   scope and dispatch NE programs to an external WineVDM installation.
6. **M5 packaging and opt-in shell integration**: ship a signed, documented
   distribution with no forced global hooks.

## Core Rules

- Runtime and tooling are C for 64-bit Windows, using documented Win32 APIs.
- The DOS VM is project-owned. NXVM can inform bounded, recorded derivations
  under its LGPL-3.0-or-later terms; it is not an unreviewed source dump.
- NTVDMx64 is a behavioral and compatibility reference only. Do not copy its
  code, private Windows symbols, loader injection, or internal-patch strategy.
- WineVDM is an optional external GPL-2.0 program launched by process boundary.
- BIOS ROMs, DOS system files, disk images, application binaries, and other
  protected media are local test inputs unless redistribution is documented.
- Every compatibility claim requires a reproducible test or an explicit
  limitation record.

Read [AGENTS.md](AGENTS.md) before changing the repository. The active work
is named in [docs/planning/status.md](docs/planning/status.md).
