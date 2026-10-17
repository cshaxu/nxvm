# Product UX

Use one Emulator Product/Session route. App supplies a two-line banner:
`MyRaidenII [0.0.NNNN]` then `Copyright (C) 2026 Xu Ha.`; Emulator supplies
the shared Built on line, startup framing and help. M0 has no runtime version.

Shared command groups/order and grammar remain authoritative:

1. start, reset, stop, pause, resume;
2. load, save;
3. debug, help, exit.

Only save/load accept their defined arguments among fixed commands. Other
fixed commands reject trailing arguments. Missing snapshot/debug capability
reports Feature not implemented., not success. Structured App extension/help
rows cannot override fixed commands or inject their own framing/prompt.

Use the shared lifecycle: initial STOPPED without a stopped notice; start to
RUNNING, pause to PAUSED, resume to RUNNING, stop to STOPPED, reset to PAUSED.
Driver prerequisites may reject a request without redefining state or text.
Window X and the chosen pause hotkey enter the same Session request path;
do not copy PC Ctrl+Alt synthetic-key behavior. Initial policy is Escape pause
toggle, with coin/start/direction/fire/bomb/service bindings frozen in M1's
selected-board input contract. Guest controls never feed cooked monitor input.
Source/focus loss and pause/reset release guest input through the existing route.

Window gameplay is the initial graphical presentation. Product injects title
identity while Emulator owns Running/Paused formatting and raw-console status.
Monitor remains usable through the existing Console broker. Existing display
and console_control policy must be tested before enabling those INI options;
booleans accept only 0/1. No graphics are sent to KVM Console as text, no
ASCII-gameplay claim is made, and an unsupported requested route must fail
explicitly rather than silently select another mode or add a second presenter.

Distinguish invalid config, missing/wrong ROM, unsupported capability and host
failure. Do not silently choose another ROM revision, fabricate guest input,
remove protection or alter game speed to pass a compatibility checkpoint.
