# T526 S11 Session Control Convergence

## Result

`vm/session` now owns the one copied fact FIFO and the lifecycle/presentation
reducer. `vm/machine` retains only its independent executor FIFO and publishes
copied results to the session-control sink. The product Console parses and
prints text, but only applies a copied session plan; it no longer decides a
surface target, title, mouse policy, hotkey action, window-close behavior, or
frame routing.

The immutable YAML-resolved request is a `vm/events` value. `vm/machine`, not
`vm/session`, translates its CPU/FPU/profile fields into Core configuration.
Thus session code contains neither Core types nor native/lib objects.

The retired product control and machine-provider adapter, their forwarding
headers, and five duplicate registrations that executed one smoke source under
different names are removed. The direct session-owner test now proves copied
latest-frame delivery and paused presentation policy.

## Verification

- `ninja -C build\\mingw-gcc-x64 -j 8`
- `ctest --test-dir build\\mingw-gcc-x64 -L unit -j 8 --output-on-failure`
  completed 299/299.
- Focused `vm-session-control-smoke` and
  `vm-product-presentation-frame-smoke` pass.

## Remaining Task Work

S12 moves the current product presentation binding into `vm/presentation`; S11
does not retain a compatibility adapter for that future move.
