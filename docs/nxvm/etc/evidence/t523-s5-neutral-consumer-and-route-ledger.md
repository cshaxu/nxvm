# T523 S5 Neutral Consumer And One-Path Ledger

## Neutral Consumer

`lib-neutral-corpus` recompiles every platform-selected C source beneath
`src/lib` into a test-only static library. `lib-neutral-consumer` links only
that target and includes only public library headers. It has no NXVM, Core, VM,
profile, firmware or asset dependency.

| Capability | Neutral proof | Existing focused proof retained |
| --- | --- | --- |
| Text/graphics frame, palette, dirty rectangle | copied `ux_frame` through `ux_mailbox` | `ux-contract-smoke` |
| Mailbox wake | wake leaves frame generation unchanged | `ux-mailbox-wake-smoke` |
| RDP-style text ingress | copied Unicode scalar `U+1F642` | `ux-linux-input-smoke` |
| Action/capture | chord registration and explicit activate/release | `ux-contract-smoke` |
| Host/lifecycle | opaque event signal/wait and paused-to-stopped state | `host-sync-smoke`, `session-state-smoke` |
| Image modes | direct-readonly, direct-writable and overlay ownership | `storage-image-smoke` |
| Outcome | copied error publication/capture | `observability-outcome-smoke` |

The neutral consumer is a second compilation of the exact library C corpus,
not a linked NXVM target being relabelled as independent proof.

## NXVM Route Disposition

| Mechanism | Sole generic route | Distinct retained product route |
| --- | --- | --- |
| Host task/event/cancel/join | `host_sync_task` / `host_sync_event` | none; VM platform no longer owns raw thread handles or flip polling |
| Lifecycle | opaque `lib_session_state` and `lib_session_executor` | VM runner retains the Core-safe pause acknowledgement and machine turn |
| Image bytes | `lib_storage_image` | FDD/HDD topology, controller requests and paired persistence remain VM media policy |
| Presenter mailbox | `ux_mailbox` is the sole native presenter cache/wake | Core presentation mailbox remains the Core-to-VM copied snapshot boundary; VM performs the one representation copy |
| Outcome | `lib_observability_outcome` | Core trace/debug state remains Core-owned, not a generic outcome duplicate |

The retained Core mailbox and runner boundary have different ownership and data
contracts; neither is an alternative native presenter, lifecycle or host-sync
path.

## Verification

- `lib-neutral-consumer`: pass (`M5:T523:S5:NEUTRAL-CONSUMER:OK`).
- Native platform run-handle proof: pass (`M5:T194:S3:RUN-EVENT:OK`).
- Full repository-only unit suite: 310/310 pass after adding the neutral
  consumer.
- Product-vocabulary, public-header and peer-root sweeps, manifest verification,
  documentation governance and `git diff --check` remain required at S5
  closure.
