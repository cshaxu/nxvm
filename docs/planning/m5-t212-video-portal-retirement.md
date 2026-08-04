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

The admitted ROM text subset is `AH=02h` cursor position, `AH=06h` scroll-up,
`AH=0Bh` palette/background compatibility, `AH=0Eh` teletype output, and
`AH=0Fh` mode query. It includes CR/LF, bounded 80x25 text progression, and
scrolling needed by POST, the no-media error message, and the retained FDD DOS
prompt. Mode setting, page selection, font, graphics, and every other `INT
10h` function are unsupported until explicitly admitted. T212 does not claim
CGA graphics support.

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
normal IVT, remove F2 registration/dispatch and QDCGA service use, and keep
state at BDA/`B8000` plus core VADP. The session runner may use a bounded
multi-instruction quantum and publish a copied display snapshot only at that
quantum boundary; command, pause, stop, and debug boundaries remain bounded by
that same quantum. Do not make `INT 10h` call platform code or add a new
firmware portal.

### S3: System-Image Regression

The new no-media regression must show the error text, reach the key wait, take
normal host input, and let the T211 report stop the session. Run FDD DOS prompt,
keyboard `ver`, VADP text, Console, debugger/pause, and two-session coverage.
The task produces `nxvm_0_5_0212.exe` only after the full matrix passes.

## S2 Result

The default ROM now owns the admitted text services through its normal IVT
entry. The implementation reads and writes only BDA state, `B8000` text
memory, and CRTC cursor ports; core VADP observes the same guest state. The
F2 video portal registration, dispatch branch, flags helper, and public QDCGA
INT 10h entry have been removed. QDCGA remains only for reset-time text-video
configuration.

The runner now executes normal guest work in fixed 256-instruction quanta and
publishes at most one copied display frame at each quantum boundary. Command,
pause, stop, and debug refresh continue before every quantum; a requested
single step still uses exactly one instruction. The no-media runner mailbox
probe reaches its text frame within one second, rejecting a return to
per-instruction full-frame copying.

## S1 Finding

The bounded probe passes within 100,000 direct core instruction quanta:
`INT 10h=20`, `INT F2h=20`, the copied VADP snapshot contains `Invalid boot
disk`, the BDA cursor is `06:00`, and the ROM reaches its `INT 16h` wait setup.
F2/VADP is therefore not the cause of the earlier ten-second observation.

The exploratory runner was slow because its current one-instruction quantum
captures and copies a full display snapshot after every instruction. That is a
separate runner cadence/performance issue, not a reason to add a video
shortcut. T212 S2 owns the bounded cadence correction as well as F2 retirement.

The FDD/DOS probe reaches the prompt within 500,000 direct core instruction
quanta and records `INT 10h=200`, `INT F2h=200`, with `AH=02h,06h,0Bh,0Eh,0Fh`.
Those five functions are the admitted S2 closure. This probe is in the FDD
smoke group so it continues to reject an unobserved portal dependency.

S2 focused verification passes: the no-media path reports `INT 10h=20`,
`INT F2h=0`, `BDA cursor=06:00`, renders `Invalid boot disk`, and reaches its
`INT 16h` wait; the FDD/DOS path reaches its prompt with `INT 10h=200`,
`INT F2h=0`, and the admitted five-function set.

## S3 Result

`current-gates-gcc` passes with all 48 current CTest smokes. This includes the
ROM no-media probe, runner-cadence mailbox probe, FDD DOS prompt and keyboard
coverage, VADP text coverage, Console lifecycle, debugger/pause, host
cancellation, and multi-session coverage. The T211 boot-failure lifecycle
smoke remains in the same matrix, retaining the normal key acknowledgement to
BDA report to session-stop path.

The task artifact is `build/output/nxvm_0_5_0212.exe`; SHA-256:
`5280BEF8C8BCBE2010C9735163B2562EB5F9EB76536484E328C3598ABD4B5BAF`.
