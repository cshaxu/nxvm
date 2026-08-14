# Windows 3.1 386 Enhanced Mode Lifecycle

## Purpose

Complete the final M5 compatibility claim for Windows 3.1 386 Enhanced Mode:
install, normal start, named ready checkpoint, and normal shutdown on the
selected 80386 PC/AT profile.

## Required scope

Consume the accepted harness, Standard Mode result, full 80386DX state
closure, and M5 L3 audit.  Run the documented lifecycle corpus and route every
failure to its earliest CPU state, interrupt/exception, paging, device, bus,
or firmware owner.  Preserve a redacted, reproducible evidence record.

## Non-goals and stop conditions

No VME/PVI or 486+ inference, x87 numeric claim, media import, guest patch,
host bypass, application breadth, or Windows 95 claim.  Stop if any result
requires a capability outside the selected profile or its approved asset
policy.

## Evidence standard

Require BYOB install/boot/ready/normal-shutdown checkpoints, deterministic
replay, state and device ledger linkage, and an explicit transfer for every
unadmitted compatibility gap.
