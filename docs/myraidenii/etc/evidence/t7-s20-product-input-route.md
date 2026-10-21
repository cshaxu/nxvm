# M1 T7 S20 — Product Input Route Qualification

## Scope and grade

S20 verifies the existing single input route; it adds no production code,
desktop automation, ROM/media retention or external implementation. The
runtime observations are **L2**. The unit assertions are local deterministic
proof of the existing contracts, not proof that the selected title accepts an
input.

## Existing route

`myraidenii_product_run()` constructs one Machine driver and passes it to the
one Emulator Product definition. The existing Product composition smoke checks
that this binding succeeds and its control/UI configuration is supplied by the
same composition. The existing Machine-driver smoke injects ordinary
`KVM_EVENT_KEY` values through that driver and verifies the complete relevant
mapping:

- `1` maps to Board Start 1;
- `5` maps to Board Coin 1;
- directions, fire, bomb and service retain their documented mappings;
- release preserves its Boolean state; unknown keys, input reset and source
  retirement do not fabricate Board controls.

There is no second Product input route, direct UI-to-Board call, or alternate
guest-control scheduler.

## Runtime result

The already-built Product executable was staged through the existing bounded
owner-ROM tool once per host width. Both runs remained alive for the full
ten-second observation and returned the existing neutral marker:

| Host width | Result |
| --- | --- |
| x64 | `MYRAIDENII:PRODUCT-LAUNCH:OK` after 10 seconds |
| x86 | `MYRAIDENII:PRODUCT-LAUNCH:OK` after 10 seconds |

The temporary directories beneath `O:\assets\raidenii\myraidenii-diagnostics`
were empty after each run. No ROM bytes, screenshots, traces, staged INI or
input payload were retained.

## Verification

- Rebuilt `myraidenii-0-0-0006` in the stable x64 and x86 Ninja trees.
- Ran all 12 registered MyRaidenII unit tests on each host width: all passed.
- Ran documentation governance and `git diff --check` successfully.

## Boundary

This establishes the existing code's Product-to-Machine-to-Board KVM route and
bounded process startup. It does **not** establish native Window focus,
physical/RDP keyboard behavior, title-screen Coin/Start acceptance, audible
output, gameplay or release qualification. Those remain T7 receivers.
