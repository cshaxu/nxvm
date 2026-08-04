# M5 T212: Video Portal Retirement

## Goal

Retire the default profile's private `INT F2h` video portal by making the
admitted text `INT 10h` subset ordinary default-ROM firmware over core VADP and
guest BDA/`B8000` state. This task does not admit graphics.

## Contract

Core VADP owns text-controller registers, `B8000` capture, dirty generation,
and copied scanout. The default profile ROM owns BIOS `INT 10h` semantics and
BDA updates. Composition copies a snapshot to platform presentation; platform
does not inspect or mutate guest video memory. No profile C callback may handle
`INT F2h` after S2; ordinary guest `INT 10h` must use the IVT and ROM `IRET`
path.

The initial ROM subset is only `AH=02h` cursor position and `AH=0Eh` teletype
output, including CR/LF and bounded 80x25 text progression needed by POST and
the no-media error message. Mode setting, scroll, page, font, graphics, and
every other `INT 10h` function stay on the existing path until explicitly
admitted or are defined unsupported. T212 does not claim CGA graphics support.

## Breakdown

### S1: Contract And No-Media Probe

Add a bounded no-media full-PC probe before guest behavior changes. It records
the ROM instruction window around the error-display path, `INT 10h`/F2
transition count, BDA cursor state, VADP dirty/snapshot state, and the terminal
wait/report checkpoint. The probe has a fixed instruction and wall-time budget;
it reports a compact verdict and retains no raw trace. Confirm the exact ROM
instructions and decide whether the admitted subset needs one additional text
primitive before S2.

S1 stops if it needs a platform shortcut, a second VADP/BDA state copy, an
unbounded trace, a CPU decoder change without a separately admitted CPU task,
or an unexplained cross-session effect.

### S2: ROM Text Slice And F2 Removal

Implement only the approved text subset in default ROM, bind it through the
normal IVT, remove F2 registration/dispatch and QDCGA service use for that
subset, and keep state at BDA/`B8000` plus core VADP. Do not make `INT 10h`
call platform code or add a new firmware portal.

### S3: System-Image Regression

The new no-media regression must show the error text, reach the key wait, take
normal host input, and let the T211 report stop the session. Run FDD DOS prompt,
keyboard `ver`, VADP text, Console, debugger/pause, and two-session coverage.
The task produces `nxvm_0_5_0212.exe` only after the full matrix passes.

## S1 Finding

The bounded probe passes within 100,000 direct core instruction quanta:
`INT 10h=20`, `INT F2h=20`, the copied VADP snapshot contains `Invalid boot
disk`, the BDA cursor is `06:00`, and the ROM reaches its `INT 16h` wait setup.
F2/VADP is therefore not the cause of the earlier ten-second observation.

The exploratory runner was slow because its current one-instruction quantum
captures and copies a full display snapshot after every instruction. That is a
separate runner cadence/performance issue, not a reason to add a video
shortcut. T212 still owns F2 retirement, but S2 must first expand its ROM
coverage decision beyond the two POST primitives enough to retain the current
FDD DOS regression.
