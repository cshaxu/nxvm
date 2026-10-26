# Product UX

XT, AT, DeskPro 386, default PC/AT and future PC110 share one NXVM interaction
model. The fixed-build/INI cutover is complete for the four implemented
products; this does not claim PC110 delivery.

## NXVM

Each executable identifies its build-fixed machine. Its product-specific INI beside the
EXE supplies startup memory, floppy/hard-disk paths and their independent
readonly/direct/overlay modes, and console/window presentation. Firmware,
CMOS seed, video ROM and font derive only from the build-fixed Profile and its
build-provided BYOB asset root. Follow SoftPC's
simple INI and existing console_control behavior, not a new configuration
framework. Absolute paths work; relative paths resolve from the INI directory.
INI boolean values accept only `0` (false) or `1` (true), including
`console_control`; textual true/false and other numeric values are rejected.
INI does not choose a machine, startup action or boot order. BIOS/CMOS remain
responsible for boot. CPU tables remain a Core capability, not an unrestricted
replacement menu. Preserve physical media-slot order, not DOS drive letters.

All machine executables use the same schema, with distinct adjacent filenames:
`my5160.ini`, `my5170.ini`, `mydeskpro386.ini` and `nxvm.ini`.
Names are declared by each App, never inferred from display names or versioned
EXE filenames. No other product's INI replaces a missing file. Omitted memory uses the
selected board default; explicit unsupported memory/media produces a clear
error. The parser must support XT sub-MiB memory without rounding it to MiB;
freeze the single memory-unit syntax during implementation planning. Firmware
roles are selected-profile facts, with external asset resolution through the
build-fixed root, never an embedded fallback, INI key or second YAML loader.

## PC110

PC110 uses the same monitor, lifecycle, input and debugger. Hardware-specific
setup belongs to its firmware and documented configuration, not another
control Console or VDM/DOS-launch interface.

The prepublication PC110 host binding reserves F12 make/break for its virtual
resume-button level. While the guest has requested firmware suspend, a new
press wakes that hardware path; it does not resume a monitor-paused machine
or send F12 to the guest. Other keys and the shared PC hotkeys retain their
normal meanings. This is an emulator binding, not an original PC110 key map;
it does not qualify a published PC110 product or a working suspend toggle.

## Presentation And Debugging

Retain Console/Window presentation, Common's existing `console_control`
behavior, lifecycle notifications, mouse policy and registered shortcuts.
Pause/resume/stop/reset remain explicit control actions. Runtime debugging and
assembly/disassembly remain in all machine builds; development trace is separate.

## Host Resources

Text cursor enable/shape and instantaneous hardware visibility are separate
Core facts. PC text frames use KVM SOURCE mode: Window draws the provided phase
without host re-blinking. Console uses native host blinking for enabled cursors
and does not promise precise hardware phase. KVM HOST mode remains available
to sources that delegate blinking; disabled cursors remain hidden.

Only supplied firmware and media enter the machine. Lib Storage owns
direct/readonly/overlay access and locking; preserve masters for overlay and
seed use. No host-drive DOS API, embedded substitute BIOS or automatic
proprietary-ROM download is introduced.
[Roadmap](ROADMAP.md) owns delivery order.
