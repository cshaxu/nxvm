# Win32 UX Native Implementation

This directory owns only Win32-native translation, geometry, pointer capture,
Console and Window loops.  It depends on `lib/ux` public value contracts; it
does not expose Windows types through them and does not include another
library component.

`actions.c` translates Win32 modifier state only.  Registered chords and their
matching have one owner in `lib/ux/actions.c`.  `input.c` translates native
packets to `ux_event`; a product binding decides their guest meaning.  The
`runner.c` is the one native Console/Window selector.  The Console lease is
the only process-global UX state; Window state is allocated per presenter run.
NXVM does not use this library until S4 can replace its existing product route
in one delivery.
