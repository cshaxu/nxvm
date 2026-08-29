# T500 S9: Integration Boundary

External-media scenarios now live below `test/integration/`. Fixed FDD/HDD
contracts are registered only under the `integration` CTest label; the unit
route remains repository-only. The migrated execution-context, default-profile,
full-session and INT13 scenarios pass with their declared configured inputs.
The Windows checkpoint has two 60-second guest-boot phases, so its one existing
integration route declares its bounded 130-second budget and runs serially;
it is not silently cut off by the generic 30-second test limit.

The final S9 verification passed the complete repository-only unit route
(312/312) and the complete external-input integration route (20/20). The
checkpoint now treats either an RTC prompt or an already-present `A:\\>` prompt
as a valid boot state, and sends the verified Set 2 `C:` sequence with the
controller cadence required by the native-input boundary.

`vm-byob-dos-boot-probe` remains a multi-profile external-firmware diagnostic;
`vm-windows31-setup-probe` remains a long-running host-observation diagnostic;
and `vm-windows31-hdd-admission-probe` retains the sole
`run-windows31-hdd-checkpoint` opt-in authority. None is duplicated or promoted
into the default integration route.
