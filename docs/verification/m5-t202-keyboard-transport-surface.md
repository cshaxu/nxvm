# M5 T202 Keyboard Transport Surface Verification

## Contract

VM platform keyboard transport has one actual role: carry host-derived guest
keypresses into session-owned ingress. Core keyboard-provider modifier queries
remain within core/profile/firmware ownership and are not re-exposed by VM
platform transport.

## Evidence

- `vm-platform-input-smoke` now proves only the retained keypress transport.
- `verify-keyboard-transport-surface` rejects the former modifier enum, query
  wrapper, and session forwarding switch while requiring the actual keypress
  surface and session ingress callback.

## Full Gate

Windows GCC current CTest passed 42/42 cases. `nxvm_0_5_0202.exe` retained the
Console banner and piped `EXIT`; SHA-256:
`B0E112E5C94240534D8E378F600DFCDB685E6C63D3A1532E7FDD1E6F8582EB27`.
