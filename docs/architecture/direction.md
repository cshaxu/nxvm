# Architecture Direction

## Product Boundary

`ntvdm64` is a DOS execution environment for 64-bit Windows, not a restoration
of Microsoft's historical NTVDM internals. Direct DOS launch constructs a
synthetic DOS process environment instead of booting a full DOS installation.
Full PC boot remains an optional diagnostic mode, not an MVP dependency.

```text
ntvdm64.exe
  command parser and executable classifier
    COM/MZ -> DOS runtime
      x86 CPU and memory
      synthetic PSP, environment, IVT, BDA, DOS services
      DOS/BIOS interrupt dispatcher
      host adapters: files, console/window, keyboard, mouse, time, ports
    NE -> external WineVDM process adapter
    PE/other -> reject or delegate to normal Windows launch
```

## Ownership

- `core/`: CPU execution, real-mode memory, interrupt dispatch, DOS process
  state, and deterministic time contracts.
- `dos/`: PSP/environment setup, MZ/COM loading, DOS service implementations,
  memory allocation, path semantics, and profile policy.
- `devices/`: text/video surface, input queues, timer, mouse, XMS/EMS, and
  optional PC hardware models.
- `host/win32/`: filesystem, console/GUI, process launch, input, clipboard,
  printer/serial policy, and diagnostics through documented Win32 APIs.
- `formats/`: MZ/NE/PE classification and safe header parsing.
- `compat/`: generated probes, manifests, expected results, and profile data.

## Non-Goals For The First Releases

- No dependence on undocumented Windows WOW/NTVDM interfaces.
- No claim to run arbitrary DOS extenders, Win16 programs, DOS games, or real
  hardware software before their specific contracts are verified.
- No bundled MS-DOS, BIOS, option ROM, commercial DOS application, or WineVDM.
- No transparent takeover of all `.exe` launches in M1-M4.

## Direct-Load Contract

For COM and MZ programs, the runtime creates the minimum documented DOS ABI
state: PSP, command tail, environment block, default handles, IVT entries,
required BDA values, load segment/registers, and an interrupt service table.
Each initialized field is a testable contract, not an incidental BIOS/POST
side effect. M1 begins with INT 20h and INT 21h AH=09h/4Ch; later services are
added only with tests.
