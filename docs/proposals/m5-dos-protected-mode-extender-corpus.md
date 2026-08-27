# DOS Protected-Mode And 386 Extender Corpus

## Purpose

Establish a small, diagnostic pre-Windows guest corpus between final selected
machine closure and the M5 pre-Windows L3 admission audit.  It makes 286 and
386 protected-mode execution observable through finite semantic checkpoints,
then uses an owner-supplied DOS extender as an integration consumer.  It is a
compatibility measurement task, not a new CPU, VM, DOS, or emulator path.

## Required scope

Freeze one ordered corpus and a local-only execution contract:

1. a minimal, source-qualified 80286 protected-mode checkpoint set;
2. a minimal, source-qualified 80386 protected-mode checkpoint set, including
   exceptions, descriptor use and return/cancellation boundaries; and
3. an optional owner-supplied DOS extender checkpoint set, initially suitable
   for a DOS4GW binary, covering its named load, protected-mode transition,
   DOS-interoperation and orderly-exit observations.

The task records the selected profile, firmware and media prerequisites, local
manifest fields, finite checkpoint matrix, output-redaction rules, timeout and
no-progress containment, and negative missing/invalid-input behavior.  Each
failed checkpoint is classified and transferred as a batch to its earliest
Core CPU, controller, display, storage, profile, or product owner.  It does
not repair the transferred issue.

The corpus may use project-owned or separately source-qualified diagnostic
programs.  A DOS4GW binary is owner-supplied BYOB input only: its bytes, local
path, hash catalogue, screenshots, guest disk images and any downloader remain
outside the repository and release.

## Non-goals and stop conditions

No bundled DOS, extender, Windows or Microsoft material; no new DOS runtime;
no inferred instruction or controller conformance; no arbitrary program
compatibility claim; and no implementation repair in the harness.  Stop and
transfer when an input lacks lawful provenance, an observation cannot be made
reproducibly, or a checkpoint needs an unproven machine capability.

## Evidence standard

Require a durable finite checkpoint ledger with prerequisites, expected
observations, actual redacted result, owner disposition and receiver for every
non-accepted row.  Require local-only BYOB rejection proof, repeated
reset/start/cancellation evidence where applicable, and an owner-visible
ready/not-ready result.  Acceptance does not replace the subsequent
pre-Windows L3 audit; that audit consumes this corpus as integration evidence
and still decides whether Windows work may begin.
