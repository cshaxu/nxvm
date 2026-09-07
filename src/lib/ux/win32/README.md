# Win32 UX Native Implementation

This directory owns only Win32-native translation, geometry, pointer capture,
Console and Window loops.  It depends on `lib/ux` public value contracts; it
does not expose Windows types through them and does not include another
library component.

`actions.c` translates Win32 modifier state only.  Registered chords and their
matching have one owner in `lib/ux/actions.c`.  `input.c` translates native
packets to `ux_event`; a product binding decides their input meaning. The
`runner.c` is the one native Console/Window selector.  The Console lease is
the only process-global UX state; Window state is allocated per presenter run.
Products adopt this library through their own binding boundary; the native
implementation never contains a product route.

Window alone applies the opaque presenter's capturable and release slots on
its UI thread. Disabling capture or requesting release calls the native release
path; clicks capture only while the latest app-provided policy permits it.
Focus loss, close and target change also release unconditionally. Console reads
neither mouse slot nor Window title.

When the sole runner changes from Window to Console, the destroyed Window is
followed by Console-surface activation in this native owner. The product does
not restore foreground or keyboard focus through a host handle.
