# Product UX

## MyNes

The cooked Console exposes SoftPC-aligned snapshot commands: `save <file>`
saves a running or paused machine and reports it paused when the capture is
complete; `load <file>` accepts only a stopped machine and reports the restored
machine paused. Both commands require exactly one ASCII path. Save failure says
that the machine state cannot be saved; load failure says that it cannot be
loaded. Snapshot output uses the same blank-line-before-prompt rule as every
other command result. A snapshot load is a control-console action: it does not
open a Window merely because the restored machine is paused.

The product has a cooked Console for management/debug and two equally supported
gameplay backends: Win32 KVM Window and ASCII KVM Console.
It has no menu, graphical settings dialog or in-game
management overlay. The Console stays available when Window gameplay runs.
App owns the command grammar, prompts, configuration and product hotkeys.

MyNes reads `mynes.ini` beside its executable before constructing Common. The
file selects the optional startup ROM and `window` or `console` presentation;
`display` defaults to `window` when omitted. Configuration is fixed for that
process lifetime, so changing it means editing the file and restarting.
The [App/Core command contract](../etc/app-core-contract.md#app-commands-defaults-and-parsing)
specifies the command grammar, ini syntax, defaults, limits and startup behavior.

Cartridge commands are `rom insert <file_path>` and `rom eject`, following the
removable-media model used for floppy insertion/ejection. Paths containing spaces
are quoted. Both use Common's existing removable-media interface; insert also
replaces an accepted cartridge, and failed preparation preserves the old one.

### Default Input Direction

The initial keyboard preset is WASD for the D-pad, J or either Ctrl for B,
K or either Alt for A, Enter for Start, and either Shift for Select. These are
fixed App defaults for the current execution profile; the monitor does not edit
bindings. KVM delivers normalized virtual keys, so the same mapping accepts
physical keyboards, RDP keyboard forwarding and virtual keyboards even when a
source does not provide a physical scan code. Cooked command editing remains
text input.
When an RDP or virtual source supplies only a text record, Core maps Return,
WASD and J/K to one executor-slice controller pulse. Text has no release event,
so it cannot express a held action; physical KVM key transitions remain the
held-input route.
Opposing directions default to neutral on that axis, recomputed when either
key is released. Held keys are levels; OS key repeat is not turbo. Turbo and
additional players need separately admitted contracts.

Game input comes only from the selected KVM surface. Typing a command never
presses guest buttons. Losing that source releases its held keys, but ordinary
focus change to the monitor does not automatically pause Window gameplay.
Rebinding clears old held state before activating the new mapping. A pause
releases gameplay state; press controls again after resuming. Input during the
short resume-cleanup boundary may be discarded. Product
hotkeys are separately registered and consumed; conflicts are rejected before
publication, including collisions with the return-to-monitor action.

## Presentation And Debugging

The Window displays the guest image and supplies normalized keyboard/mouse
events. Mouse presence in the KVM contract does not imply an NES mouse or light
gun device. Ordinary NES play uses the admitted controller mapping. Display
selection and sizing are command/default policy, not a second UI settings owner.

Management and debug share the cooked reader and output path. The primary
SoftPC-style monitor verbs are `start`, `resume`, `pause`, `stop`, `reset`,
`help` and `exit`. Debugging is a nested App grammar (`debug regs`, `debug mem`,
`debug step` and related commands), never a collection of competing top-level
verbs. Help and configuration inspection can work while running;
machine-dependent inspection/edit/step requires paused state and otherwise
reports that requirement. No separate debug executor or second native Console
reader is permitted.

Cooked editing may wait for a full line while the machine continues to run.
A runtime notification interrupts an unfinished edit through controlled reader
cancellation, prints once, then presents a fresh prompt; unfinished text is
discarded with an explicit notice. A completed queued command is consumed once.
Logs and frame notifications do not print directly over a partially typed line.

### ASCII KVM Console

ASCII gameplay displays the normal game image as character cells, with optional
cell colors. Core supplies that text rendition; kvm-console displays text frames
only. It presents the same running machine without changing NES hardware mode.
The `display` setting in `mynes.ini` selects Core's published representation at
startup; Common continues its ordinary frame-kind-driven mode switching.
Conversion and routing ownership are defined in [Architecture](ARCHITECTURE.md).
publisher is parked during paused debugging.
The chosen representation takes effect on explicit start/resume because the shared
publisher is parked during paused debugging.
publisher is parked during paused debugging.
Both presenters are required product outcomes. M3 qualifies both with real
Win32 native receivers: Window reads presented pixels and Console reads its
active text buffer after Core conversion. Future compatibility work does not
remove either presentation route.

When ASCII gameplay owns the native Console, it uses raw input. Cooked management
and raw gameplay take turns on the same Console through one broker. Esc is the
reserved pause/resume hotkey and controller remapping cannot consume it. Pausing
returns the Console to its cooked prompt and scrollback; `resume` restores its raw
presenter. Window mode retains an already-open Window while paused and the cooked Console stays
the control surface. Switching a presenter alone neither resets nor silently resumes
the machine.

## Host Resources

File selection is explicit through startup or Console commands. Core hardware
receives bytes and typed options; its media integration owns cartridge loading through Lib storage. One broker owns native Console mode,
reader and output restoration. KVM Window has an independent native lifetime.
Changing surface releases held inputs and retires the old reader before another
starts. Closing a KVM surface returns control to the monitor at a paused boundary;
process exit remains an explicit command or host process-close action.
A guest trap remains paused for inspection/reset/replacement. Host ERROR allows
help/exit and requires restart. `reset` is a power reset that clears
volatile RAM; paused `debug reset` preserves RAM. Loading never autoruns or creates a Window.
Media provenance and redistribution follow the
[source policy](../etc/operations/policy/source-policy.md).
