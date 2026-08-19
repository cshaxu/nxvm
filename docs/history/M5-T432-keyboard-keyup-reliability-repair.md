# M5 T432: Keyboard Key-Up Reliability Repair

## Task Record

T432 fixes the owner-reproduced Model-339 stuck-key and typematic regression.
Core owns the separate delayed-translation and native-break typematic state;
VM host ingress and both presentation modes remain shared consumers.

## Verification

Focused KBC controller smoke, Model-339 clock contract smoke, current smoke
gate build, and owner Console/Window manual confirmation pass. The developer
artifact is `vm-0-5-0432`.