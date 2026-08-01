# M5 T12 S7 First Retained Producer Switch Design

The first producer is `win32KeyboardMakeStatus()`, called by Console KEYUP and
FOCUS events and window KEYUP/FOCUS events. It only synchronizes modifier and
lock state; key-down translation and the historical F9 behavior remain direct.

The producer captures `GetAsyncKeyState` modifier bits and `GetKeyState` toggle
bits into one copied `KEYBOARD_STATE` ingress request. At the next execution
boundary, the adapter-owned consumer applies the full snapshot before later
FIFO requests. If ingress is full or closed, the producer invokes the retained
direct status function as a compatibility fallback.

Teardown closes ingress before the platform owner stops, clears the callback
only during full-PC destruction, and discards after stop. Focus/key-up ordering,
Console/window input, FDD/HDD boot, Console/debugger, and no-recorder gates are
required before the switch can close. No runtime code changed in this design.
