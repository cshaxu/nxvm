# M1 T7 S11 — External MAME Reference Prerequisite

## Scope and method

This is one finite external-reference observation.  It does not execute or
retain MAME source, ROM contents, a snapshot, a trace, audio, or a screenshot.
It cannot establish local emulation correctness, timing, title, gameplay,
sound or video qualification.

The existing external executable was
`O:\\repos.external\\mame-0.289\\mame.exe`; its `-listfull raiden2` identifies
the target as `Raiden II (US, set 1)`.  The owner-supplied archive was read
only from `O:\\assets\\raidenii\\raiden2.zip`, SHA-256
`9FEDC8DDAC2208408C3B05201316E3F2DB43BDFB104551BA17337BB55DB13D71`.

MAME was given that directory as its ROM path, a ten-second wall-clock
`-seconds_to_run` limit, `-nothrottle`, `-sound none`, and `-video none`.
All MAME cfg, NVRAM, snapshot, state and input output roots were freshly
created below `O:\\assets\\media-myraidenii\\t7-s11-mame\\`.

## Result

The process returned exit code 2 before executing the game.  Its loader
reported these two required files absent from the selected archive:

- `jj4b02__ami18cv8-15.u0342`
- `jj4b01__mmipal16l8bcn.u0341`

The selected archive's existing JED identity is not a substitute for either
binary PAL dump.  No file was downloaded, manufactured, renamed, copied into
the archive, or otherwise used to bypass this requirement.

The temporary external root was deleted after the process exited; the
repository and owner archive were not modified.

## Evidence grade and disposition

This is **L2** only: it is an observed outcome of a third-party reference
binary with a different declared input inventory.  It does not establish that
the two PAL dumps are required by the physical local product implementation,
nor does it indicate a local Core/Machine defect.  It does prove that this
particular MAME-run experiment cannot be used as a startup differential until
lawful owner-supplied PAL dumps are available.

The local selected-board product must retain its independently verified input
contract.  Do not add PAL paths, configuration fallbacks, synthetic PAL data
or a MAME runtime dependency merely to make this reference observation start.
