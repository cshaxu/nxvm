# M6 Mantle Experiment Program

After M5 closes, the following four candidates establish the M6 source-locked,
single-session mantle experiment. They do not define a DLL/SDK or external ABI.
Mantle assembles one `core_machine` from typed profile and host providers
supplied by a VDM adapter; it does not own DOS behavior, host path policy,
product UI, or a VM profile. The adapter remains responsible for connecting DOS
and host-facing providers.
