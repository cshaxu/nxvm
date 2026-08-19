# M5 Core L3 Integration Closure And VM Contract Export

## Purpose

Integrate the reconciled timing plan, CPU programs, transaction lifecycle and
device-phase contracts into one Core L3 capability surface. Publish the stable,
validated contract catalog that a later VM profile resolver may select, without
implementing that resolver or changing a profile in this task.

## Required Scope

Consume all preceding Core candidates and their closure ledgers. Reconcile the
master ledger against the actual Core API, default plan and regressions; prove
that every public capability has one L3 contract, explicit L2 fallback, not
applicable or unsupported disposition. Export only neutral contract IDs,
versions, validation requirements and observability declarations. VM machine
identity, inheritance, provenance and YAML policy remain outside Core.

## Dependencies

Last in the ordered Core L3 sequence. A later VM/profile program may consume
its exported catalog only after this candidate accepts it.

## Evidence And Completion Standard

Require end-to-end Core composition tests across the supported CPU and shared
controller configurations, immutable-plan and rejection tests, trace checks,
all current gates, and an independent ledger-to-code closure audit. The audit
must prove no profile name or profile-local timing algorithm entered Core and
no public capability lacks a declared disposition.

## Non-goals And Stop Conditions

Do not claim every historical machine is L3, implement profile inheritance or
user YAML, admit new CPUs/devices/chipsets, import firmware/media, or perform
L4 work. Stop if a remaining gap belongs to VM/profile ownership; transfer it
explicitly to that later program.
