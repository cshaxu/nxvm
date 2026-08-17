# DeskPro 386 C1 Semantic Endpoint Investigation

## Purpose

Establish whether the selected deterministic Model-40 startup route has an
existing-state, finite C1 semantic endpoint beyond M40-C0A, or exposes one
reproducible earliest-owner functional defect that prevents such an endpoint.
This is the narrow successor to T391 and the sole path toward re-admitting the
blocked 80386 physical-retirement candidate.

## Required Scope

Freeze a finite inventory of candidate transitions or externally observable
semantic milestones available through existing copied retirement, device and
session state. For each candidate record reset/failure semantics, ownership,
consumer relevance, and whether it can be proven without raw PC/bytes/traces
or a new Core/VM callback. Use bounded owner-managed replay only as diagnostic
input; no budget is a completion condition. If the inventory identifies a
reproducible functional defect, first record its exact shared owner, callers,
variants, regression and whole-class sweep before any repair. Otherwise transfer
the complete candidate batch and leave physical qualification blocked.

## Completion Standard

The task finishes only with a durable finite candidate ledger and one of:
(1) an accepted, existing-state checkpoint with reset/failure proof and a
project-owned regression; (2) a bounded earliest-owner defect repair with its
complete sweep and regression; or (3) an explicit complete transfer showing no
admissible C1 endpoint. It never turns a containment count into a checkpoint.

## Boundaries

No firmware/media import, raw trace retention, ROM catalogue, Core/VM ABI
expansion, VM-specific Core callback, timing allocation, physical-clock
selection, board/device timing, firmware-compatibility or L3 claim. Third-party
emulators are read-only secondary diagnostic references only.