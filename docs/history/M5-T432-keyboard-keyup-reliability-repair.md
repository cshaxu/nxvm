# M5 T432: Keyboard Key-Up Reliability Repair

## Task Record

T432 fixes the owner-reproduced Model-339 stuck-key and typematic regression.
Core owns the separate delayed-translation and native-break typematic state;
VM host ingress and both presentation modes remain shared consumers.

## Verification

Focused KBC controller smoke, Model-339 clock contract smoke, current smoke
gate build, and owner Console/Window manual confirmation pass. The developer
artifact is `vm-0-5-0432`.
## Closure

P1 `113ec4ef` delivers the repair. Owner manual verification confirms `dir` plus
Enter and ordinary characters no longer repeat in either Console or Window
display. The T432 artifact is `vm-0-5-0432`, SHA-256
`F29A7DF2BA6F43D2A7E1B4C9AABCD634123E4E57F92226796B5734B4A71C413C`.