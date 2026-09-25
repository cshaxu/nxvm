# Source, License, And Research Detail

This supports [Architecture Rules](../../../../rules/ARCHITECTURE.md); it cannot
admit a new dependency, product feature or redistribution commitment.

## Project And NXVM Provenance

MyNes uses the root MIT License, Copyright (c) 2026 Neko, as requested by the
owner. The governance migration derives from the MIT NXVM revision recorded
in [provenance](../../provenance/nxvm-governance.md). Preserve that notice.
Importing governance does not import NXVM runtime code or its historical tasks.

For every imported or substantially derived unit, record exact source revision,
source/destination paths, license/notices, modifications and verification.
An independent third-party notice requires separate review before copying.
Sibling repositories remain read-only unless the owner authorizes changes.

## Reference Implementations And Manuals

Other emulators and CPU cores are behavior/design references until an explicit
source/license review admits reuse. Do not copy or transliterate incompatible
code. Hardware claims name primary documentation and reproducible probes where
available; reference behavior alone is not automatic specification authority.
Store acquired manuals outside tracked source; evidence may identify source,
edition, acquisition date and digest without committing the document.

## ROMs And Test Fixtures

Existing and future owner-managed game ROMs remain local ignored inputs.
They are not source, default dependencies, repository fixtures or release
payloads. A filename or possession does not establish redistribution rights.
New unit tests use project-owned or separately license-reviewed redistributable
fixtures. External-ROM scenarios have explicit admission, bounded execution,
expected observations and cleanup ownership under the integration subtree defined by Source Layout.
Never commit machine-local paths, protected ROM bytes or raw unbounded traces.

## Shared Corpus Admission

The owner selected SoftPC Lib/Common as the initial reusable source corpus.
This repository now publishes the canonical Shared baseline; owner-approved
Shared changes require neutral contracts, manifests and receiving-App verification.
Each import still records pinned units, API/dependency fit, licenses/notices,
transferred tests and one update owner. Separate unchanged units from derived
units; retain source hashes/manifests and explain necessary deltas. Source reuse
authorization is not a license finding: a missing root license does not establish
MIT rights over every unit, especially recovered-machine code. Review per-unit
provenance and resolve any missing grant before importing affected source.
Do not rewrite compatible mechanisms merely to avoid an inventory. Do not make
a sibling checkout a runtime/build dependency or modify it without authorization.

The neutral source/test transfer consists of the four roots plus
`test/register.cmake`, as mapped by [Source Layout](../../../design/CODING.md).
The optional x86 source/test pair extends it to six roots. Its target is one neutral corpus
reimportable unchanged into SoftPC/NXVM; product-specific debugger integrations
are outside it. Record root mapping, source/test hashes, generic changes and
consumer adapter migration separately. An initial snapshot is not a declaration
that every inherited module is already neutral or compatible.

Source/test copying is deferred until the relevant milestone is admitted, its
local T decomposition/queue is reviewed and an import task is admitted under
Execution. Approval of roadmap boundaries alone permits no source copying. No documentation approval alone authorizes a source import or sibling edit.
