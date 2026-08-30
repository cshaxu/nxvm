# M5 HDC Personality Service-Deadline Closure

## Purpose

Replace the T504 HDC L1 compatibility candidate with sourced service/deadline
relations for each supported fixed-disk personality: default-AT ATA PIO,
IBM-5170 WD1003/ST-506, DeskPro Compaq/WD and IBM-5160 Xebec DMA. Retain one
Core HDC owner and one media boundary.

## Scope And Constraints

This task does not equate ATA with MFM/ST-506, Compaq/WD or Xebec, and does
not claim physical media/ECC/mechanics without their own evidence. The HDC
alone owns command phases, DRQ, IRQ, result consumption and cancellation;
media supplies bytes/persistence only. DMA timing is consumed from its earlier
receiver, never recreated inside HDC.

## Required Sequence

1. Admit and quality-check each selected controller/personality source and
   board fact; cross-check corresponding paths in 86Box, MAME, PCjs, Bochs and
   QEMU without treating them as authority.
2. Freeze a complete List 1 for command, data/DRQ, IRQ/result, reset/cancel,
   DMA/PIO handoff and observable service/deadline behavior per personality.
3. Produce List 2 by tracing all rows through the sole HDC and media/IRQ/DMA
   consumers; identify no alias or duplicated phase state.
4. Implement the whole sourced owner-local batch, leaving every unavailable
   service value explicitly classified rather than guessed.
5. Run personality-focused tests, complete unit and affected disk/ROM/DOS
   integration regressions.

## Exit Criteria

Each selected personality has a complete evidence and code disposition, and
the HDC T504 candidate is retired only where its normal phase transition has a
valid deadline or an explicit non-eligible boundary. No second HDC, controller
alias, media cache or VM timing path exists.
