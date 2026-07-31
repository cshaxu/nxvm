# M5 T2 S2 PC/AT Builtin Verification

The canonical machine id `nxvm.machine.pc_at_builtin` and provider id
`firmware.provider.pc_at_builtin` registered through the session registry and
resolved as a compatible builtin pair. The focused smoke then created the
retained baseline adapter from the local FDD fixture and observed reset vector
`F000:FFF0`, emitting `M5:T2:S2:PC-AT-BUILTIN:OK`.

The established full-PC FDD/HDD profile smoke also emitted
`M3:T3:S1:FULL-PC-PROFILE:OK`. `git diff --exit-code -- src/nxvm-baseline`
passed. No firmware bytes, external ROM loader, media policy, or guest behavior
changed.
