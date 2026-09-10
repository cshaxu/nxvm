# M5 T526: NXVM Canonical-Library Integration Repair

## Active Packet

- **Identifier mode:** New
- **Owner admission:** repair NXVM/lib integration defects while preserving the
  admitted canonical SoftPC `src/lib` corpus byte-identical.
- **Current subtask:** S3 - lifecycle/Console coherence.

## S1 Contract

The direct reproducer is a 25-row text frame whose rows from approximately 13
through 25 have invisible text in both Console and Window.  Core produces
8-bit attributes while `ux_frame` stores 16-bit attributes; a byte-counted
copy populated only the first 1,000 of 2,000 UX elements.  S1 replaces that
invalid representation copy with explicit element conversion and audits the
remaining Core-to-UX copied fields for the same class of width/count error.

The regression must prove the final 80x25 cell's character and attribute
survive conversion.  `src/lib` is excluded from modification: SoftPC's working
canonical presenter proves this is a product-adapter fault.

## S2 Contract

The canonical lib already turns `WM_CLOSE` into one copied `UX_EVENT_WINDOW_CLOSE`
and deliberately does not destroy the native surface.  NXVM must consume that
event as a distinct product close intent, not as the pause-toggle event:
running closes pause at the next safe boundary and then remove the Window;
paused sessions merely remove the Window; resume recreates the requested
Window surface.  No Window close may stop or resume a session implicitly.

## S3 Contract

SoftPC's monitor reports `Machine paused.` and `Machine stopped.` only after
the corresponding runtime transition completes.  NXVM must establish the same
one-way product route: session runner acknowledges the lifecycle state;
NXVM's product Console retains and formats the resulting completion.  `src/lib`
remains unaware of session state and no runner writes host text directly.  The
product queue is bounded and its overflow is an explicit Console failure, not
a silently dropped completion.

A Console-mode pause releases the borrowed native Console so the NXVM command
Console can report the state and accept `RESUME`, but it must remain a paused
session.  Reclaiming Console on resume is presentation work, not a new session
or reset.  Window close retains the S2 contract and is one producer of the
same paused completion.

A Console pause ends only the synchronous presenter runner after the lease is
released.  It does not emit `stopped`; the later `STOP` command owns that
completion when no runner remains.

### Owner-approved canonical-refresh amendment

The owner admitted canonical SoftPC revision
`7cc408ec2e27c8243ae2a3f719d313bb7e851e1d` into S3 after NXVM-governance
audit.  Replace `src/lib` byte-for-byte, including its public
`*_interface.h` boundary; retain no compatibility header or local lib patch.

For NXVM's multi-session product, use SoftPC's separation without copying its
single-runtime policy: each composition session owns execution and actual
lifecycle facts, while one VM-product control owner receives copied command,
UX, lifecycle, frame and delivery-failure facts through a bounded FIFO.  It
tags each asynchronous producer with session identity and run generation,
rejects stale input, derives presentation actions, and is the sole Console
formatter.  A session-local presentation binding applies those actions through
the canonical lib and returns completion facts to the product FIFO.  Core
continues to own guest state and snapshots only.

## Exit Boundary

S1 closes only after the complete repository-only unit suite passes.  T526
closure additionally requires its S2/S3 binding sweep, external integration,
canonical manifest and boundary gates, and fresh stripped x64/x86 0526
artifacts.

### Owner-approved product-control migration amendment

The owner further required NXVM to adopt SoftPC's lifecycle and product
experience separation, while retaining NXVM's multi-session capability.  The
next S therefore replaces, rather than wraps, the mixed `vm/platform` UX
route: sessions publish copied facts, process-owned product control owns
identity/generation-aware decisions, and per-session product presentation owns
the only lib leaf.  Core and the shared library remain outside product policy.
