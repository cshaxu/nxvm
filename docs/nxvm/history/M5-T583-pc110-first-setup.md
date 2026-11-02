# M5 T583 — MyPC110 first configuration and restart

Status: closed after S1–S9. The owner requested completion of the first four
PC110 receivers; this is the first of those receivers. Its retained proposal
is [here](M5-T583-pc110-first-setup-proposal.md), and the detailed evidence is
the [coverage ledger](../etc/evidence/t583-first-setup-ledger.md).

Both actual x86 and x64 products, with derived DSTN and TFT firmware, passed
first Setup, normal CMOS save, independent process restart and DOS boot. TFT
was tested both with an absent file and with a truly all-zero CMOS. Deliberate
date edits on both widths and both variants, and deliberate hour edits on
DSTN x86 and TFT on both widths, were retained across real-process restart.
All four combinations returned to DOS after stop/start and reset/resume, with
actual POST frames observed. The DSTN firmware is a derived image and is not
claimed to be a verified original dump; running-time RTC behavior is not a
claim that elapsed time while the emulator is stopped is reconstructed.

The x64 and x86 complete repository unit suites passed 712/712. The PC110
external firmware integration suite passed 9/9 on each width. Documentation
governance and diff checks passed. T583 changed only NXVM evidence and state
documentation; no production, Shared source, tests, external media or
published EXEs were changed. The actual product artifacts and task-private
disk hashes are recorded in the ledger.

The initial DSTN x86 lifecycle observer returned a nonzero result after its
final pause, without distinguishing timeout from process exit. An independent
complete repeat reached DOS and exited zero, but cannot establish the cause of
the earlier observation. This uncertainty transfers to the subsequent product
interaction receiver; it is not described as a fixed product defect. Native
message tests were executed on isolated desktops; live interactive desktop,
RDP, broader device behavior and final product qualification remain with the
subsequent three PC110 receivers. T582's PC110 startup-performance debt and
the separately recorded X86 Core masked-access debt remain open.
