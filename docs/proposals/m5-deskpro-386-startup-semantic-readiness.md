# DeskPro 386 Startup Semantic Readiness

## Purpose

Establish the bounded, non-timing startup-semantic prerequisite needed before a
selected external-ROM DeskPro Model 40 corpus can supply a later C1 CPU
qualification stage. This candidate is inserted before renewed 80386 physical-
retirement qualification because the latter needs a meaningful finite startup
checkpoint, while the existing board-timing candidate correctly requires
physical qualification first.

## Required Scope

Starting from the accepted Model-40 ROM mapping, A20 startup policy, selected
logical device topology and owner-managed BYOB contract, define one finite
startup checkpoint expressed only through existing or narrowly admitted copied
state. It must distinguish an observed startup semantic transition from a
retirement budget, wall-clock cap, raw ROM trace, PC/byte capture, or guessed
POST phase. It may repair a bounded earliest-owner functional defect only after
an owner/source/caller sweep, focused regression and similar-issue review.

The candidate may use the existing Core retirement-observation lifecycle and
VM composition contracts, but must not add a VM-specific Core callback, expose
mutable Core state, create a second boot path, or use external firmware as a
repository dependency. The selected Model-40 profile remains deterministic;
the checkpoint is diagnostic input and cannot publish physical CPU, board, or
device time.

## Non-goals And Stop Conditions

No Intel instruction timing qualification, physical-retirement selection,
board-clock ratio, DMA/PIT/READY timing, firmware/media import, raw trace,
firmware compatibility claim, physical-device timing, or Model-L3 decision.
Stop and transfer when the required semantic fact cannot be represented by
bounded copied state, when it belongs to a wider device/firmware mechanism, or
when an external asset is needed as more than an owner-managed transient input.

## Evidence And Completion Standard

Require a source/owner/caller inventory, one named finite semantic checkpoint,
project-owned regression, reset and failure-boundary proof, a contained
owner-managed replay when available, and full current-gate verification after
any code change. The closure must state whether the checkpoint makes C1
admissible, transfers a functional defect, or proves that renewed physical
qualification remains blocked. It never claims that a captured boot phase is
physical time or complete firmware execution.