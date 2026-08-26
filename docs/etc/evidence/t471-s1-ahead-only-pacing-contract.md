# T471 S1: Ahead-Only Pacing Contract

`M5:T471:S1:AHEAD-ONLY-PACING-CONTRACT:OK`

## Exact Decision

For a copied verified observation at Core tick `E` with rate `R`, a session
captures one baseline `(E0, H0, F)`: Core tick origin, monotonic host-counter
origin and host-counter frequency. Standard is ahead exactly when

```text
(E - E0) / R > (H - H0) / F
```

where `H` is a later monotonic host-counter sample. This is a comparison only:
it does not calculate a guest tick, write Core state, compensate a lag, or
make a profile physical. S2 must implement it with an overflow-safe rational
comparison, not cross multiplication. A host-counter regression or changed
rate discards the baseline and takes no Core action.

## HLT Rule

Core alone supplies a copied `next_deadline_tick`. For Standard, an HLT
deadline may be requested from Core only when

```text
(deadline - E0) / R <= (H - H0) / F.
```

Until then VM waits for host progress; it never advances guest time to make the
comparison true. Turbo may request the same Core-selected deadline immediately.
If the axis/deadline is unavailable, existing L2 behavior remains unchanged.

## Lifecycle And Proof Seam

- The pacing baseline belongs to one `vm_session`, initializes only from a
  verified copied observation, and resets on successful Core reset, stop,
  cancellation or invalid host sample.
- Pause, debugger command handling and display cadence remain host-control
  paths, not pacing writers.
- The pure comparison receives explicit tick/rate and host-unit/frequency
  values in a focused test seam. Tests cover equality, lead, lag, near-maximum
  values, counter regression, reset, unavailable axis and HLT deadline policy.
- No current profile has a verified axis, so product smokes continue proving
  only the unavailable L2 path. Synthetic Core axis fixtures prove the new
  comparison without promoting a product profile.

## Source Sweep

T470's `vm_session_waiting_advance` currently captures the copied observation
but immediately calls `core_machine_advance_to_next_deadline` whenever the
axis is verified; this is the exact corrective receiver. `runner.c` has the
single unavailable Standard 1 ms backoff. `virtual_time.c` currently exports
millisecond display cadence from a high-resolution platform counter, so S2
must expose a separate precise monotonic observation rather than reuse a
rounded value. No other VM caller converts host elapsed time into Core ticks.
