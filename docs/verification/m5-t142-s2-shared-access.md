# M5 T142 S2: Bounded Shared-Device Access

## Result

`vm_session` no longer stores PIC, PIT, DMA, KBC, or VADP raw aliases. The
composition access record borrows each capability from its one `core_machine`
only at the binding point. FDC receives those direct same-object bindings;
default profile creation receives VADP directly. Neither path copies state nor
acquires a lifecycle role.

## Verification

Windows GCC built and ran:

```text
M5:T83:S2:CORE-EXECUTOR-STORAGE:OK
M5:T70:S1:P5:FDC-CONNECT:OK
M5:T40:S1:QDCGA-BOUNDARY:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T70:S2:DOS-PROMPT:OK
```

Production and test source scan finds no raw `vm_session` shared-device member
access. T142 S3 audits debugger target capability use and the completed access
boundary.
