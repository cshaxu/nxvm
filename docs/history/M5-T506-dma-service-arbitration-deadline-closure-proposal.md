# M5 DMA Service And Arbitration Deadline Closure

## Purpose

Replace the T504 DMA L1 compatibility candidate with source-qualified selected
8237A service/deadline behavior wherever the chip and board evidence support
it. Cover only the existing bound refresh, FDC and XT Xebec consumers through
the sole Core DMA owner.

## Scope And Constraints

DMA remains the owner of request, arbitration, DACK, transfer mode, terminal
count, EOP, masking, cascade and reset. Bound controllers own their own command
state; profiles only freeze board clock/wiring inputs. A missing service value
remains explicit L1/L2 as its evidence requires: this task may not add a
per-device timer, DMA shim, VM tick source, or parallel bus path.

## Required Sequence

1. Admit and quality-check Intel 8237A plus selected XT/AT board material;
   compare the selected routes with 86Box, MAME, PCjs, Bochs and QEMU.
2. Freeze List 1 for every selected request/service/arbitration transition,
   including PIT1 refresh, FDC DMA2 and Xebec DMA3, with reset/cancel and
   provider-consumer ordering.
3. Produce List 2 by tracing each row through the sole DMA owner and its Core
   deadline/publication path.
4. Implement the full owner-local batch for every source-qualified relation
   and retain labelled lower-evidence rows without guessed timing.
5. Run DMA/provider tests, complete unit and affected ROM/DOS integration
   regressions; transfer no unclassified selected row.

## Exit Criteria

Every selected DMA service relation has a manual/source-backed deadline,
labelled lower-tier disposition, or explicit unsupported receiver. The DMA
T504 candidate is retired only when its normal owner lifecycle, reset,
withdrawal and downstream FDC/HDC/refresh effects are all preserved.
