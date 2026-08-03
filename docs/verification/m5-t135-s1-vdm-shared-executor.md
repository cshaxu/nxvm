# M5 T135 S1: VDM Shared Executor

`vdm_machine_dos_minimal` now creates `CORE_MACHINE_PROFILE_CUSTOM` and uses
the core-owned CPU, bus, and memory preparation contract before installing its
own keyboard, timer, and text-snapshot providers. It owns no CPU, RAM, or port
mirror. Core reset and destruction now reset/finalize executor memory and ports
regardless of whether VM shared devices are installed.

This remains a non-DOS-runner skeleton: it has no DOS loader, BIOS/DOS service
implementation, or VDM command path.

Verified with:

- `ntvdm64-vdm-minimal-session-smoke`
- `nxvm-dos-minimal-presentation-smoke` (including executor-dispatched port
  `0x60` reads for two isolated VDM sessions)
- `nxvm-vm-dos-prompt-smoke D:\\home\\repos.hobby\\fdd.img`

The developer artifact is `build/output/nxvm_0_5_0135.exe`
(`5630A3B3628B66F29B8B6B888D05D710772FE213F165B11348D2F2C4EAEB562A`).
