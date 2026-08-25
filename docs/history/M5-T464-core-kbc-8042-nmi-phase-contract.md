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
