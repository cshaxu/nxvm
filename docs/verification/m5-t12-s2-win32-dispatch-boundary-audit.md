# M5 T12 S2 Win32 Dispatch Boundary Audit

`platform/win32/win32.c` receives host events from both `win32con.c` and
`win32app.c`. It directly writes every BIOS keyboard modifier/lock flag through
`deviceConnectKeyboard*`, reads those flags while translating key codes, sends
the resulting key through `deviceConnectKeyboardRecvKeyPress`, and calls
`deviceStop` for F9. These calls occur on the Console/window host path, so a
direct source move would preserve forbidden platform-thread guest mutation.

The compatible bridge has three machine-execution-boundary requests: keyboard
state snapshot, key press `(scan_code, virtual_key)`, and stop request. The
platform owns Win32 collection and submits copied requests; machine-owned code
observes guest keyboard state and applies the existing translation at the
command boundary. Display mode changes are similarly copied mode requests and
are consumed by the platform presentation owner, rather than being called by
firmware/QDX directly.

The bridge must be introduced inactive alongside the retained direct path, with
scripted Console/debugger and Windows input/display smoke gates before any
producer is switched. This audit made no source changes, enabled no recorder,
and did not run a guest trace.
