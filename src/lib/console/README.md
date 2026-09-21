# console

`console` provides the platform-neutral `lib_console` logical Console object,
copied input/output values, and lifetime/output contracts. It depends only on
`types` and `base`. `console-broker` binds one caller-owned logical Console to native I/O;
`kvm-console` creates one for its raw Console lifecycle. This component contains
no native Console handle, platform mode, monitor, or application lifecycle policy.
Its Base mutexes block competing metadata/callback/output
operations without busy-waiting. Sink callbacks cannot synchronously reenter
binding replacement or destruction; detach remains a quiescence barrier.
The adapter-only `binding_interface.h` installs one copied output binding:
text callback, text-frame callback and a shared borrowed context. Replacing it
with NULL clears both. The output gate covers either callback and replacement;
after replacement returns, no previous output uses the old context. A valid
object always accepts replacement; an absent callback makes its write return
NOT_CURRENT. Input generation and its separate event gate are unchanged.

Text-frame cells contain copied BMP Unicode values, not encoded byte indices.
Separate foreground/background arrays contain palette indices 0..15, not native
attribute words. Invalid visible colours are rejected before calling the sink.
The native renderer forwards them without code-page interpretation. Character
mapping belongs to the producer; this component stores no encoding table or
bitmap font and cannot reproduce arbitrary uploaded fonts.
