# M5 T200 Keyboard Ingress Boundary Verification

## Contract

`vm_session` owns request ingress and is its only consumer. VM platform code
submits host modifier/toggle snapshots through the run-context operation;
session consumption at the runner command boundary is the sole route to the
core keyboard provider.

## Focused Evidence

- `vm-request-transport-smoke` proves two adjacent keyboard-state snapshots
  coalesce to the latter one, while a key press separates snapshot positions.
- `vm-keyboard-host-ingress-smoke` reads the QDKEYB BIOS flags: a submitted
  state leaves them unchanged before the boundary, then applies left-shift,
  control, and caps-lock only after it. The retained Win32 key ingress remains
  queued until its later boundary.
- `verify-keyboard-ingress-boundary` emits
  `M5:T200:S1:KEYBOARD-INGRESS-BOUNDARY:OK`; it rejects the old direct VM
  callback, direct Win32 sink access, and loss of session-side consumption.

## Limits

This task does not change the core keyboard provider, QDKEYB mapping, KBC
controller behavior, or Linux native runtime verification.

## Full Gate

Fresh Windows GCC `current-gates-gcc` passed 41/41 CTest cases and all static
targets. `nxvm_0_5_0200.exe` retained the Console banner and piped `EXIT`
behavior; its SHA-256 is
`9EEFAF6477764749F104FA0B1DA7120E86A58576110F75F4E814E9CFD3542BFA`.
