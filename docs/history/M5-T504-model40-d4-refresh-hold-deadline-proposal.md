# M5 Model-40 D4 Refresh-Hold Deadline Closure

> Retained proposal for the completed T504 task.

## Purpose

Replace the T504 Model-40 D4 refresh-hold L1 compatibility candidate with the
selected DeskPro 386 board relation where original or qualified board evidence
supports one. The result belongs solely to the existing Core D4 owner.

## Scope And Constraints

This is not a generic DMA refresh model, a new Model-40 scheduler, or physical
cycle reconstruction. D4 retains refresh-hold state, transaction consumption,
cancellation and reset. Generic DMA retains its own state; profile composition
may provide only frozen source-backed board input at construction. VM and
firmware cannot observe or advance D4 privately.

## Required Sequence

1. Admit and quality-check original Compaq/board material; inspect 86Box,
   MAME, PCjs, Bochs and QEMU only as corroborating implementations.
2. Freeze List 1 for D4 refresh request, hold visibility, interaction with the
   selected memory/DMA route, cancellation and reset.
3. Produce List 2 by tracing each row through the existing D4 and Core
   progression owners and every affected profile consumer.
4. Implement the complete D4-local correction batch only for sourced timing or
   immediate ordering; preserve a labelled lower-tier boundary otherwise.
5. Run D4/Model-40 focused tests, complete unit and affected external-ROM/DOS
   integration regressions.

## Exit Criteria

The D4 T504 candidate has an evidenced immediate/deadline/lower-tier
disposition with reset and cancellation proof. No D4 state leaks into DMA, VM,
firmware or a second scheduler, and the selected Model-40 route remains one
owner-local mechanism.
