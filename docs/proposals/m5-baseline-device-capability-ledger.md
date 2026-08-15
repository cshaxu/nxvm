# M5 Baseline-Machine And Supported-Device Capability Ledger

## Purpose

Freeze the capability and ownership ledger that precedes all three remaining
baseline-machine L3 closures. It identifies the exact selected configuration
and required device surface of IBM PC/AT 5170 Model 339/Type 3, the 1986
original Compaq DeskPro 386 Model 40 (DeskPro 386/16), and IBM PC/XT 5160-268,
then reconciles those selections with every device capability the product still
publicly exposes. This is an audit and task-allocation candidate, not device
implementation or an L3 decision.

## Required scope

For each baseline, record its documented board/revision, CPU and clock,
memory/ROM slot constraints, selected video, input, storage, interrupt/DMA,
timer, NMI, reset, and probe-corpus boundaries. The 5170 record retains its
8 MHz Type-3/Rev.3/512 KB/CGA/no-fixed-disk baseline and treats a documented
3.5-inch 1.44 MB drive only as an aftermarket field upgrade, not an IBM
factory configuration. The DeskPro and XT records must not fill unspecified
values with generic clone or PC/AT assumptions.

For every selected component and every currently exposed product capability,
classify its status as complete, partial, empty, test-only, or unsupported;
then name the register/state-machine, IRQ/DMA, reset, ownership, timing, and
consumer gap plus its earliest implementation receiver. Distinguish shared
semantic repair from profile-specific binding and timing. A capability outside
all three baselines may remain supported only if the later current-product
closure owns it; otherwise it must be removed from the supported surface before
M5 closes.

## Non-goals and stop conditions

No code repair, timing value, ROM/media import, new public device, generic
machine profile, or L3-ready claim. Stop at any undocumented configuration or
unobservable dependency and record a bounded receiver or supported-surface
decision; do not borrow a value from 86Box, MAME, PCjs, or a different board.

## Evidence standard

Require a source-labelled machine bill of materials and a single
machine-to-device/product-capability matrix. Each row must identify its
functional state, owner, earliest receiver, later timing/audit receiver, and
primary-source/probe need. The record must prove that no final L3 audit can
mistake an absent function for a timing-only gap.
