# Component Inventory Scope

Research must inventory the complete possible environment rather than assume
that `NTIO.SYS`, `NTDOS.SYS`, and `COMMAND.COM` form a standalone backend.

Candidate groups include `NTVDM.EXE`, `NTVDMD.DLL`, guest DOS files, ROM and
VGA ROM files, `HIMEM.SYS`, `DOSX.EXE`, redirection and debugging components,
`CONFIG.NT`, `AUTOEXEC.NT`, WOW components, and Windows host facilities.

For each item, record version, architecture, legal acquisition constraint,
hash, role, startup dependency, guest-or-host classification, public/private
ABI dependency, and whether it can plausibly run outside the original host.
