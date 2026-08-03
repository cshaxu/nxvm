# M5 T124 S1: VDM Session Authority

`vdm_machine_dos_minimal` is VDM-owned. `vdm_session` owns it and exports the
bounded input, text, snapshot, and port capabilities used by presentation.
Presentation no longer accepts or retains a raw minimal-machine object.

MinGW-w64 GCC 16.1.0 passed VDM minimal-session, presentation-isolation, and
DOS-minimal-profile gates. No VDM runtime loop or CLI was added.
