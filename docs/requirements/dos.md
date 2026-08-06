# DOS Backend Requirements

M7 delivers a deliberately small, project-owned DOS profile. It is not an
MS-DOS version claim and it does not promise behavior outside this document.

## M6 Design Gate

Before M7 implementation, this profile is completed as a testable ABI: COM load
segment and image limits; PSP, environment, DTA, stack, and initial registers;
`INT 20h`/`INT 21h` entry and return; register/FLAGS and memory effects;
handle allocation; read, seek, EOF, pathname, input-blocked, error, and exit
semantics. Every allowed service has an executable vector. Intentional absences
are specified here, never inferred from implementation.

## Program Model

- Load one COM image at a fixed, documented conventional-memory location.
- Create a PSP with command tail, environment block, default DTA, and standard
  handle slots required by the approved services.
- Do not load MZ images, implement `EXEC`, or start an implicit command shell.
- `INT 20h` and `INT 21h/AH=4Ch` terminate the guest and return its exit code
  to the DOS harness.

## Approved INT 21h Subset

| AH | Service | M7 behavior |
| --- | --- | --- |
| `02h` | Write character | Append `DL` to deterministic standard output. |
| `09h` | Write `$` string | Append `DS:DX` bytes through `$` to standard output. |
| `0Bh` | Check keyboard status | Return `AL=FFh` when input is queued, otherwise `AL=00h`. |
| `01h` | Read character with echo | Consume one queued byte or stop as input-blocked; echo it. |
| `3Dh` | Open | Open an approved 8.3 root fixture path read-only (`AL=00h`). |
| `3Eh` | Close | Close a fixture-file handle. |
| `3Fh` | Read | Read from an open fixture-file handle. |
| `42h` | Seek | Seek an open fixture-file handle within the fixture file. |
| `4Ch` | Terminate with code | End the guest with `AL` as its exit code. |

All other `INT 21h` functions fail with `CF=1`, `AX=0001h` (invalid function).
For approved file services, defined failures use DOS-style `AX` values: missing
file `0002h`, invalid handle `0006h`, and invalid drive `000Fh`. Each service
records register preservation and success/failure state in its focused test.
An `AH=3Dh` access mode other than read-only fails with `CF=1`, `AX=0005h`.
For `AH=42h`, M7 supports `AL=00h` (offset from file start) only; a requested
position outside the fixture file or another origin fails with `CF=1`,
`AX=0001h`.

## Memory Boundary

M7 provides fixed loader, PSP, stack, environment, DTA, and COM image memory
only. It does not implement `INT 21h/AH=48h`, `49h`, or `4Ah`; therefore it
does not create an MCB chain. Dynamic DOS memory, MCB behavior, MZ loading, and
`EXEC` belong to M10 unless a later owner-approved requirement changes the plan.

## Filesystem Boundary

M7 uses a host-independent in-memory fixture filesystem with one `C:` root
volume and uppercase 8.3 file names. It accepts `C:\\NAME.EXT` and
`NAME.EXT`; any other drive is invalid. It has no directories,
current-directory changes, wildcards, FCB, device names, long file names, or
host paths. Fixture contents are created by project-owned tests; no host
filesystem API may be called from `dos/`.

M9 replaces the fixture adapter with a Platform-owned host filesystem adapter.
The M9 adapter, not M7, enforces `--drive`, `--hide-drive`, canonicalization,
reparse-point handling, and UNC/device-namespace denial.

## Console Boundary

M7 Console input and output are deterministic harness queues and buffers. They
have no Win32 Console, GUI window, pipe, Ctrl+C, mouse, or host-redirection
semantics. Those are Platform product behavior in M9.

M7 test execution uses a project-owned developer/debugger load path and a
project-owned fixture adapter. Loading is accepted only while the machine is
paused, debug mode is enabled, and no program has been selected or loaded; it is
rejected otherwise. After success, it remains paused before the first program
instruction; only a reset returns to the loadable state. The load path must pass
the selected image into the COM loader; it must not inject bytes at an arbitrary
CPU address or bypass PSP/environment/loader semantics. It does not implement
the `nxvdm run` product path, host-drive mapping, or product command-line
parsing. Its host path is a developer/test input only; DOS sees only the
fixture filesystem and cannot enumerate or open that host path.

## Acceptance Probes

1. A COM hello probe uses `AH=09h`, terminates through `AH=4Ch`, and verifies
   captured output and exit code.
2. A COM file probe opens a seeded fixture, reads and seeks it, closes it, and
   verifies bytes, file position, and exit code.
3. A negative COM probe verifies invalid-function, missing-file, and
   invalid-handle `CF`/`AX` results.
4. A deterministic keyboard probe verifies queued input, empty-input stop, and
   echo behavior.

## Stop Rule

An unlisted DOS service, pathname feature, device behavior, memory API, or host
integration request does not enter M7 as an incidental fix. Record evidence and
route it to M9/M10 or propose an explicit owner-approved DOS-profile amendment
with a new acceptance probe.
