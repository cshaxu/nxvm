# M5 DOS Backend Requirements

M5 delivers a deliberately small, project-owned DOS profile. It is not an
MS-DOS version claim and it does not promise behavior outside this document.

## M4 Design Gate

Before M5 implementation, this profile is completed as a testable ABI: COM load
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
  to the M5 harness.

## Approved INT 21h Subset

| AH | Service | M5 behavior |
| --- | --- | --- |
| `02h` | Write character | Append `DL` to deterministic standard output. |
| `09h` | Write `$` string | Append `DS:DX` bytes through `$` to standard output. |
| `0Bh` | Check keyboard status | Return `AL=FFh` when input is queued, otherwise `AL=00h`. |
| `01h` | Read character with echo | Consume one queued byte or stop as input-blocked; echo it. |
| `3Dh` | Open | Open an approved 8.3 root fixture path read-only (`AL=00h`). |
| `3Eh` | Close | Close an M5 fixture-file handle. |
| `3Fh` | Read | Read from an open fixture-file handle. |
| `42h` | Seek | Seek an open fixture-file handle within the fixture file. |
| `4Ch` | Terminate with code | End the guest with `AL` as its exit code. |

All other `INT 21h` functions fail with `CF=1`, `AX=0001h` (invalid function).
For approved file services, defined failures use DOS-style `AX` values: missing
file `0002h`, invalid handle `0006h`, and invalid drive `000Fh`. Each service
records register preservation and success/failure state in its focused test.
An `AH=3Dh` access mode other than read-only fails with `CF=1`, `AX=0005h`.
For `AH=42h`, M5 supports `AL=00h` (offset from file start) only; a requested
position outside the fixture file or another origin fails with `CF=1`,
`AX=0001h`.

## Memory Boundary

M5 provides fixed loader, PSP, stack, environment, DTA, and COM image memory
only. It does not implement `INT 21h/AH=48h`, `49h`, or `4Ah`; therefore it
does not create an MCB chain. Dynamic DOS memory, MCB behavior, MZ loading, and
`EXEC` belong to M8 unless a later owner-approved requirement changes the plan.

## Filesystem Boundary

M5 uses a host-independent in-memory fixture filesystem with one `C:` root
volume and uppercase 8.3 file names. It accepts `C:\\NAME.EXT` and
`NAME.EXT`; any other drive is invalid. It has no directories,
current-directory changes, wildcards, FCB, device names, long file names, or
host paths. Fixture contents are created by project-owned tests; no host
filesystem API may be called from `dos/`.

M7 replaces the fixture adapter with a Platform-owned host filesystem adapter.
The M7 adapter, not M5, enforces `--drive`, `--hide-drive`, canonicalization,
reparse-point handling, and UNC/device-namespace denial.

## Console Boundary

M5 Console input and output are deterministic harness queues and buffers. They
have no Win32 Console, GUI window, pipe, Ctrl+C, mouse, or host-redirection
semantics. Those are Platform product behavior in M7.

M5 test execution uses the retained NXVM Console `load` workflow and a
project-owned fixture adapter. It does not implement the `ntvdm64 run` product
path, host-drive mapping, or product command-line parsing.

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
integration request does not enter M5 as an incidental fix. Record evidence and
route it to M7/M8 or propose an explicit owner-approved M5 profile amendment
with a new acceptance probe.
