# M5 T11 S12 QDX Ownership Boundary Audit

The retained QDX package is legacy built-in firmware, not a machine-neutral
device: `qdx.c` dispatches synthetic interrupt stubs; `qdkeyb.c`, `qddisk.c`,
and `qdcga.c` implement INT 09h/16h, INT 13h, and INT 10h behavior. Its forward
owner is therefore `profile/vm/default_profile/firmware`, not `machine/core`.

`qdcga.c` has four direct calls to `platformDisplaySetScreen()`. Moving it to
`machine/core` would violate the platform boundary; moving it unchanged to a
profile would retain a forbidden concrete platform dependency. `qdx` is also a
single dispatch table shared by all four files, so moving only one handler
would create a duplicate or split dispatcher.

The safe order is: migrate the VM platform hub and retain presentation behavior;
introduce the profile-to-presentation request boundary; then move the complete
QDX dispatcher/handler package as one coherent firmware slice. This audit made
no source changes, enabled no recorder, and did not run a guest trace.
