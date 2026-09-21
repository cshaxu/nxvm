# Product UX

Standard and PC110 share one NXVM interaction model. This target design does
not claim that both executables are delivered.

## NXVM

Each executable identifies and boots its fixed machine. YAML supplies
firmware/media paths, access modes and supported options, not machine topology.
Preserve simple YAML, relative-to-YAML and absolute paths, ordered media slots
and clear load failures. CPU tables remain a Core capability, not an unrestricted
CPU replacement menu for a fixed product.

## PC110

PC110 uses the same monitor, lifecycle, input and debugger. Hardware-specific
setup belongs to its firmware and documented configuration, not another
control Console or VDM/DOS-launch interface.

## Presentation And Debugging

Retain Console/Window presentation, Common's existing `console_control`
behavior, lifecycle notifications, mouse policy and registered shortcuts.
Pause/resume/stop/reset remain explicit control actions. Runtime debugging and
assembly/disassembly remain in both builds; development trace is separate.

## Host Resources

Only supplied firmware and media enter the machine. Lib Storage owns
direct/readonly/overlay access and locking; preserve masters for overlay and
seed use. No host-drive DOS API, embedded substitute BIOS or automatic
proprietary-ROM download is introduced.
[Roadmap](ROADMAP.md) owns delivery order.
