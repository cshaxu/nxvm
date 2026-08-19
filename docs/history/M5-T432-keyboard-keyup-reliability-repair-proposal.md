# Keyboard Key-Up Reliability Repair

## Purpose

Repair the reproduced Model-339 stuck-key regression. A complete Set-2 key-up
sequence reaches the KBC, but delayed Set-2 translation and typematic
cancellation shared one break-prefix state. The final native byte could restart
typematic before translation consumed its prefix.

## Bounded Approach

Core keeps separate private state for delayed translation and native-stream
typematic cancellation. VM host ingress, profile timing and display modes are
unchanged.

## Proof And Limits

The focused controller smoke proves a serially delayed Set-2 `b` make/break
emits Set-1 `30h/B0h` and leaves typematic inactive. The Model-339 timing
contract passes, and the owner manually verified Console and Window behavior.