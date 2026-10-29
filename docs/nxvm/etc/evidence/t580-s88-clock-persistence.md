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
