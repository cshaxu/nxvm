# T580 S88 — PC110 RTC persistence investigation

The existing PC110 CMOS file stores configuration RAM at 0Eh–7Fh. The
Machine capture contract explicitly zeroes 00h–0Dh and seed application
deliberately starts at 0Eh. Those bytes therefore do not preserve the RTC
calendar across Machine destruction and reconstruction.

A task-owned probe derived from the existing PC110 machine construction test
used the normal debug port route to write BCD year 24h to RTC register 09h,
captured the existing CMOS image, destroyed the machine, and constructed a
new one with that image. Both x64 and x86 read 24h before capture and 00h
after reconstruction. This is an in-process construction experiment, not an
independent product-process or actual guest test. It did not modify user CMOS
or media. The normal RTC reset implementation retains calendar and phase;
this static observation must not be confused with process-restart proof.

The RTC's owner advances its calendar from guest ticks, not host wall time.
Persisting the last displayed calendar alone would improve restart behavior
but would not establish battery-backed elapsed time while the program is not
running. Any design must preserve the guest-set clock and distinguish these
two obligations. The existing configuration-only CMOS contract and its
callers require review before any Shared interface changes.

The existing PC110 construction test now also writes year 24h through the
machine driver, performs its real reset callback and reads the same RTC year
back. This existing fixture passes on x64 and x86 after an incremental Ninja
build (0.98 s and 0.16 s test execution respectively). It establishes reset
retention at the machine boundary, not guest firmware or process-restart
behavior.

The public Lib Base clock API explicitly supplies monotonic time only; it is
not a cross-boot wall clock. The RTC advances once per elapsed guest second
in a loop, so blindly replaying a long powered-off interval would also have
unbounded startup cost and could raise update/alarm flags. A correct design
must specify powered-off elapsed time separately and avoid that replay path.

## Interface boundary for review

The existing 128-byte file and `capture_cmos` are configuration contracts;
changing their interpretation for every PC profile would silently affect
other machines. A candidate is an explicitly optional RTC calendar seed in
the existing Machine construction data and a quiescent RTC calendar capture
from that same Machine. Board and RTC remain the only owners of live state;
the App supplies saved values at construction and persists a copy only after
the worker stops. Register C cannot be read for persistence because its read
acknowledges interrupts. Restore must honor the RTC's binary/BCD and 12/24
hour modes, rather than interpreting raw calendar bytes independently of
register B.

An elapsed-time model additionally needs a cross-process wall-clock source.
The existing Base monotonic counter cannot supply it. A possible neutral Lib
Base API is UTC seconds on Win32 and Linux; a PC110-owned file can store the
last guest RTC and the corresponding host timestamp. Old 128-byte files must
remain accepted as configuration-only inputs, and newly written data must be
explicitly versioned and validated. A backward host clock must not subtract
time from the guest; a host clock read failure must remain a reported error.
Offline calendar advancement must be bounded independently of the number of
seconds, must respect the RTC divider and SET state, and must not synthesize
normal running interrupt events by calling the existing per-second advance
loop.

This is a design candidate, not authorization or a completed repair. It
would change Lib Base and X86-NXVM public interfaces and their tests, in
addition to the PC110 App and its tests. It also addresses only the process
restart interval: RTC advancement while stopped or paused and the interaction
with the unpaced Turbo guest clock require separate evidence before claiming
full battery-clock behavior. No source or test from the external emulator is
copied; its host-clock behavior is a research lead only.

## Running and paused clock ownership

Static source tracing shows that PC110's RTC domain uses 32768/33000000 of
Core ticks, and `board_advance.c` is the sole caller of `x86_rtc_advance`.
PC110 selects Turbo by default; the Machine runner does not pace Turbo Core
execution to host time. Consequently the emulated RTC follows guest execution
time rather than a guaranteed host second in Turbo. During a pause that stops
Core time, the RTC likewise has no independent advancing owner. Standard mode
does pace guest time, but that fact alone does not prove accuracy under host
overload or during suspension. These are static conclusions about the existing
time path, not measured wall-clock drift or a completed fix. A restart-only
timestamp adjustment must not be represented as fixing these runtime cases.

A separate x64 task-private construction probe read the RTC seconds register,
waited 2,100 ms without executing the Machine, and read it again: `00,00`.
The probe completed successfully and used task-owned files. This confirms the
quiescent Machine observation, not an end-to-end Product pause/resume run;
Turbo drift has not been measured.

## Existing released-product observation

Reinspection of the S87 uninstrumented TFT x64 product captures found a
visible time discrepancy: World Clock showed `2000-01-01 00:00:23` in frame
24 and `2000-01-01 00:00:25` in frame 25. The observer requests those captures
in a loop with a 20,000 ms sleep between them; their file timestamps also
differ by approximately 20 seconds. The frame hashes are respectively
`1C29BDF7A55BDA61F22CFDB33BD62BBBFB4669B1D54B3905BB27695966B19F61` and
`CC7D0C6CCBCC0265431C622950F14D7BA269A40063CA1474855422FC2FB7D2BA`.
The actual observer binary in that run hashes to
`AED4FEE7D7A3766EDA1EA10B000D11B3BF36E9D451953C0B5615192B9B1B6663`.
The retained `released-ready-probe.c` is consistent with its creation time
and has the 20,000 ms capture loop; disassembly of the binary also contains
the 20,000 immediate in its capture loop. The source/binary identity was not
reproducibly rebuilt, so that correlation is not a byte-for-byte build proof.
This is a guest-application display observation, not a measurement of the
RTC registers or proof that RTC alone causes the slow clock: DOS timer
delivery, guest scheduling and host performance also require examination.
The date shows what this isolated guest displayed, not whether its owner had
previously set a different calendar.
