# T588 S2: actual reset proof and bounded execution baseline

Baseline: `786c4d993`, 2026-11-19. No production changes. Both existing
Release Ninja trees report no work for the immutable-ROM regression target;
both executions pass `IMMUTABLE-ROM-MAPPING` and `ROM-PREFETCH-BOUNDARY`.

## Reproduction and limits

Read the S1 hash-verified common dump. Copy offset `0x7f0000`, length 65536
into a temporary diagnostic buffer, register immutable mapping at `0xf0000`
and reset-window alias at `0xffff0000`. Create the existing neutral executor
with 2 MiB RAM, 80386, no FPU and ticks_per_instruction=1. Install a port
provider which records the first access and returns UNSUPPORTED rather than
inventing device behavior. Freeze, reset, get copied CPU state and run one
instruction per call, bounded by 100,000 calls and 120 seconds. Stop at the
first port, zero-retirement result or error. Use public Core interfaces only.

The mapping and CPU selection are diagnostic hypotheses, not qualified OB300
hardware. The same source links against the current x64/x86 cached
X86 Core Machine/CPU/FPU libraries with `-O3 -std=c11 -Wall -Wextra -Werror`.
It creates no alternate executor, production API or shared test dependency.

Both widths give exactly:

| Step | CS:EIP | Linear address | Result |
| --- | --- | --- | --- |
| 0 | F000:FFF0 | FFFFFFF0 | Original far jump executes |
| 1 | F000:E05B | 000FE05B | Next original instruction executes |
| 2 | F000:E05C | 000FE05C | Next original instruction executes |
| 3 | F000:E05E | 000FE05E | Write byte 0 to port 00FBh; diagnostic declines |

The provider returns UNSUPPORTED (3); the run boundary reports status 6
(INTERNAL_ERROR), reason 7 (FAULT), zero retirement for that call, elapsed
ticks 18. Do not confuse the callback result with the execution result.
Three earlier instructions retired. This proves
the first real BIOS route is executable; it does not identify port FBh's
hardware function or prove the entire 64 KiB map. HP board initialization,
not a demonstrated unsupported CPU opcode, is the first required receiver.

## Fixed-work envelope

In a separate fresh Machine, put NOP/JMP-back at F000:0000 and a reset far
jump to that address. Run a 10,000,000-instruction budget without port logging
or per-instruction copied-state reads. Both return OK/budget, exactly
10,000,000 retired instructions, PC F0001 and 55,000,005 elapsed ticks.

| Cached Release width | Process-inclusive elapsed seconds |
| --- | --- |
| x64 | 7.2376703 |
| x86 | 8.3525357 |

One run each; Python perf_counter brackets the whole process, including launch,
allocation and teardown. These are baseline observations, not statistical
estimates, physical 20 MHz qualification or predicted ROM boot time. No device
timing or missing-port wait was suppressed. The old PC110 comparison is not
an apples-to-apples speedup denominator.

## Local primary and software evidence

Owner's local HP service manual 5965-0244 SHA-256:
`3787092f86f252846605d2d5b45d7712eeb79345cc9c4ef1eaae64ccaa34aa09`.
Printed page 68, PDF page 74, was both extracted and visually checked. Its
300/425 column places VGA RAM in A/B, UMB in C, PCMCIA in D, ROM DOS in E,
and VGA/system BIOS within F. The diagram does not specify bank-register
values or justify mapping the VGA ROM at the usual C0000 address. It also
does not justify treating the probe's whole F segment as final system BIOS.

The dump at 0x7e0000 starts with a valid option-ROM signature and declares
32 KiB; that region's byte sum modulo 256 is zero. Its SHA-256 is
`8d432a26c9650d025ae02b56c5d79b2f52982400c9e1cb92212e4aeb0f047550`.
The candidate 64 KiB system slice at 0x7f0000 has sum 244, not zero; its
upper 32 KiB sums to 64. No simplistic whole-slice BIOS checksum conclusion
or card corruption claim is warranted.

The matching System 1.0 Drive C ZIP passes CRC. CONFIG.SYS selects the D-drive
shell and HIMEM /machine:2; SYSTEM.INI names vgarom3.drv, the D-drive mouse
driver and ROMUpdates. Preserve this pairing rather than substituting generic
Windows files. It remains a file backup, not proof of the original sector map.

## Review and handoff

O01/O02 now have actual generic-386 reset proof under a named hypothesis;
O03/O04 retain decoder and masked-span questions; O05/O07 require HP port and
input wiring; O11 has a fixed-work baseline. No row is silently promoted to
complete hardware qualification. S3 must turn all twelve rows into explicit
design/model choices and receiver tests, including FBh and the F-segment split.

Scratch source hash: `790dc09ea06609ef266f0a41340ddf4438ac6280cb452658dfb066f7e73fcfe0`.
Runner hash: `1aab5d17223e137c84868ab5492a9fc1174edbd3fe26fc3eafafc37f0740d45c`.
Result hash: `cd65d36923a5d65e7d6d6dc791185f1545299bfa7468873535de7b493a4935a6`.
Required reproduction parameters and outputs are retained above; temporary
ROM slice, rendered manual and probe artifacts are removed after process exit.
Sandbox background launch produced no run; the persistent hidden rerun above
is the evidence. No desktop/RDP, full boot, full-suite or product-release claim.
Only documentation changes: no EXE refresh or unrelated regression required.
