# console

`console` provides the platform-neutral `lib_console` logical Console object,
copied input/output values, and lifetime/output contracts. It depends only on
`types`. `host` binds one caller-owned logical Console to native I/O;
`ui-console` creates one for its raw Console lifecycle. This component contains
no native Console handle, platform mode, monitor, or application lifecycle policy.
Its private win32/linux mutex implementations block competing callback/output
operations without busy-waiting. Sink callbacks cannot synchronously reenter
binding replacement or destruction; detach remains a quiescence barrier.


Text-frame bytes use a fixed PC-display glyph mapping (CP437 graphics, including
the low graphic symbols; zero is blank). `lib_console_pc_glyph` maps each byte
to Unicode without inspecting an application's encoding. This is not bitmap
font storage and cannot reproduce arbitrary uploaded fonts. Native renderers
use Unicode cells instead of filtering bytes to ASCII.
