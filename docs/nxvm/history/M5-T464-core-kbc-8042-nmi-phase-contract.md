# M5 T464 Core KBC 8042 NMI Phase Contract

## Active Record

T464 owns the queued 8042/KBC/NMI phase contract. Its initial source ledger
reconciliation must complete before any implementation batch is admitted.

## S1 Accepted Evidence

`f1275be9` records the complete sixteen-row reconciliation. Intel UPI-42 and
IBM 5170 rendered PDFs are normative. 86Box, Bochs, PCjs, MAME and QEMU are
read-only cross-checks: they corroborate register, command, IRQ1 and
A20/reset models, but disagree on scheduler details and therefore do not
manufacture a hardware tick rule. The retained gaps are one UPI-style input
service phase, selected status/input/test/diagnostic state, and explicit
board-timing boundaries; no KBC NMI owner or external source import is allowed.

## S2 Accepted Evidence

`8319dd91` removes the duplicate `system_flag` mirror: the command byte is the
sole status-bit owner. It adds only the selected input/test state and C0h/E0h
command cases to the existing KBC path. C0h, E0h, inhibit status/override and
the existing AUX/serial paths have focused passing smoke proof. `AC` is not
faked: its 60 scan-code dump requires selected 8042 RAM, PSW and ROM firmware.
Serial error latches, UPI service cadence and output-pulse consumption remain
explicit L2/L4 boundaries.

## Closure

S3 built the stripped Release 0464 artifact (SHA-256
`92BF67903AF9F7725F9BFC8182373FDF615DD5026EDA1857C87BF8249912F674`) and
ran the isolated serial current gate successfully: 294/294 in 113.34 seconds.
The retained S3 audit maps all ledger rows and confirms no duplicate KBC state
or path was introduced.
