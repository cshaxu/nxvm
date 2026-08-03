# M5 T120 S1: VM Session Initialization Authority

`vm_session_initialize` now returns immediately for an active session. This
keeps the retained Console target's historical initialize callback valid for a
zero-initialized direct session while making `vm_session_create` the one
initialization authority on the executable path.

The session smoke verifies that an explicit second initialize preserves the
same core machine and control object. Console lifecycle and DOS-prompt gates
passed with MinGW-w64 GCC 16.1.0:

- `nxvm-product-session-smoke`: `M5:T7:S1:NXVM-SESSION:OK`
- `nxvm-product-console-lifecycle-gate`: `M5:T96:S1:CONSOLE-LIFECYCLE:OK`
- `nxvm-vm-debug-target-smoke`: `M5:T14:S3:VM-DEBUG-TARGET:OK`
- `nxvm-vm-dos-prompt-smoke D:/fdd.img`: `M5:T70:S2:DOS-PROMPT:OK`

Developer artifact: `build/output/nxvm_0_5_0120.exe`, SHA-256
`7CB61CADB277DC5F56E4585435583C667C7FBA585F9AFB3905668A14835A3F18`.
