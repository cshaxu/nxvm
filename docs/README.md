# Documentation Guide

This is the sole documentation entry point.

## Task Reading Set

Before changing work for an S, every participant reads:

1. this guide;
2. the active [Status](STATUS.md) packet and its S brief;
3. the [Execution Rules](rules/EXECUTION.md) reading index and the lifecycle
   paragraphs it names; and
4. [Contributing](../CONTRIBUTING.md).

The active packet or brief may name additional authorities, local instructions,
evidence, or artifacts; those are also required. Then apply these triggers:

- A coordinator planning or admitting an S also reads [Queue](QUEUE.md) and
  [Roadmap](design/ROADMAP.md). When closing an S, the coordinator also reads
  [Documentation Rules](rules/DOCUMENT.md).
- A code or build change reads [System Architecture](design/ARCHITECTURE.md),
  [Source Layout](design/CODING.md), [Architecture Rules](rules/ARCHITECTURE.md),
  and [Coding Rules](rules/CODING.md), plus any affected local guidance.
- A documentation or governance change reads
  [Documentation Rules](rules/DOCUMENT.md), every authority it directly
  changes, and every authority whose normative requirements the change alters.
  A UX change also reads [Product UX](design/UI.md).
- Importing, deriving, researching, or packaging source, firmware, guest
  media, Microsoft material, or third-party code first reads the
  [source and research policy](etc/operations/policy/source-policy.md).

This is the task-reading authority. Do not treat the following orientation map
as a requirement to re-read every authority for every S.

Read the fixed set when first starting an S and on its first resume. Re-read an
authority when the packet, S brief, that authority, or the change surface has
materially changed; routine implementation steps and progress reports do not
require a full repeat reading. This cadence does not excuse reading every
authority newly triggered by a revision or discovered change surface.

## Orientation Map

For system-wide orientation, read the current authorities in this order:

1. [Project Goal](design/GOAL.md)
2. [System Architecture](design/ARCHITECTURE.md)
3. [Source Layout](design/CODING.md)
4. [Product UX](design/UI.md)
5. [Roadmap](design/ROADMAP.md)
6. [Documentation Rules](rules/DOCUMENT.md)
7. [Execution Rules](rules/EXECUTION.md)
8. [Architecture Rules](rules/ARCHITECTURE.md)
9. [Coding Rules](rules/CODING.md)

## Daily Operation

`STATUS.md` is the only active-task and technical-baseline authority.
`QUEUE.md` contains ordered, unnumbered candidate work; only an approved active
task receives a numeric identifier. `TODO.md` contains unplanned debt only.

## Supporting Detail

[etc/README.md](etc/README.md) indexes owner-approved supporting material,
including detailed contracts, verification material, research, provenance,
fixtures, release material, templates, and legacy documentation. It cannot
define a competing current architecture, rule, task queue, or active status.
