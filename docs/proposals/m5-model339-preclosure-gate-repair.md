# Model-339 Preclosure Input And Global-Gate Repair

## Purpose

Repair the shared production-boundary defects exposed by T377 before a
Model-339 L3 acceptance is reconsidered: native Win32 keyboard events must
reach the controller only through production transport ingress; native mouse
events must likewise obey the platform transport boundary; and the published
virtual-time/composition dependency boundary must pass its static gate.

## Scope and non-goals

Use production-native ingress, not a test-only injection API. Preserve the
frozen Model-339 IRQ1-only and no-AUX selection while retaining the shared AUX
transport for its later product-level closure. Repair the dependency edge at
its true architectural owner. Do not add media, ROMs, new machine variants,
ATA/MFM behavior, physical timing claims or a Model-339 L3 acceptance.

## Evidence and completion

Require a caller/handler/rollback sweep for native keyboard and mouse ingress,
focused production-path regressions, dependency-gate replay and a passing
configured current gate. Record selected Model-339 effect, retained
product-level AUX boundary, and reset/cancellation behavior. The following
Model-339 re-audit is the sole L3 decision receiver.
