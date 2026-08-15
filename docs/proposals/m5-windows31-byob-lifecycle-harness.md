# Windows 3.1 BYOB Lifecycle Harness

## Purpose

Provide a legal, reproducible, owner-supplied Windows 3.1 observation harness
only after the 5170, DeskPro 386, 5150/XT 8088 baseline-machine, and
current-product device-capability L3 closures and the pre-Windows L3 admission
audit are accepted. Windows runs
against the frozen selected 80386 DeskPro 386 profile.

## Required scope

Define a BYOB manifest and local-only runner for owner-supplied installation
media and disk images.  Record profile, firmware slot constraints, media
hashes outside the repository, scripted checkpoints, timeout/no-progress
limits, log/redaction policy, and cleanup.  The harness must distinguish setup,
first boot, interactive-ready checkpoint, and normal guest shutdown without
committing Microsoft binaries, paths, screenshots, or guest images.

## Non-goals and stop conditions

No bundled media, downloader, default runtime dependency, inferred hardware
support, or opaque "it booted" claim.  Stop if a required media/legal boundary
or reproducibility contract cannot be documented under the source policy.

## Evidence standard

Require an owner-supplied manifest schema, deterministic runner contract,
negative missing/invalid-media proof, and a redacted checkpoint record.  This
task establishes measurement only; it does not claim either Windows mode works.
