# M5 Core L3 Ledger Reconciliation And Conformance

## Purpose

Create the one durable Core L3 master ledger by reconciling, not repeating, the
project's retained CPU, transaction, device and capability ledgers. Then check
the current public Core implementation against that ledger. This candidate
establishes the factual migration baseline for the remaining Core L3 program.

## Required Scope

The frozen input corpus includes T346 Core-machine/device L3 audit, T354
transaction ledger, T357 instruction-timing contract and profile ledgers,
T359/T363 timing corpus, T369 bus-timed PC/AT operation, T370 device service
corpus, T373 baseline capability ledger, T377 5170 audit, T401 CPU correctness,
T404 current public device capability ledger, T420 CPU execution audit, and
later retained DeskPro evidence where it changed a shared Core owner.

For every current public Core capability, record: neutral capability ID, code
owner and caller boundary, historical source ledger/evidence, selected
L3 contract or L2 fallback, reset/cancellation lifecycle, regression owner,
and current conformance disposition. Permitted dispositions are conformant;
implementation discrepancy with the earliest receiver among the next five
Core candidates; explicit accepted L2 fallback; not applicable; or unsupported.

This task must inspect the current code and tests, not merely merge documents.
It must not re-prove an already accepted historical fact or silently change a
historical result.

## Dependencies

This is first in the Core L3 sequence. It consumes retained evidence and
precedes all Core timing-plan, CPU, transaction, device and integration work.

## Evidence And Completion Standard

Publish an indexed finite master ledger and a conformance report. Every
currently public Core CPU, transaction, memory/route, controller, device and
integration capability has exactly one disposition and named evidence/code
owner. Every discrepancy has one downstream Core candidate or TODO receiver;
no gap may be hidden by a successful boot or a generic timing default.

## Non-goals And Stop Conditions

Do not implement a timing engine, repair behavior beyond an immediately unsafe
publication defect, alter a VM profile, repeat past audits, import external
material, or assert L4. Stop and transfer a contradiction between retained
ledgers rather than selecting a winner without authority.
