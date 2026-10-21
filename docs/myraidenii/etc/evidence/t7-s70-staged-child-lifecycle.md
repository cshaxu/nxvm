# T7 S70 — Staged Child Lifecycle

## Scope

This record covers only wrapper process ownership and two independent test
presentation fixes.  It retains no ROM content, guest trace, address, opcode,
screenshot, third-party source or protected media.

## Result

The selected-set wrapper previously invoked Windows-subsystem children with
PowerShell's direct invocation operator.  PowerShell could then return before
the GUI child exited, while the child continued using its staged external
asset directory.  The wrapper now owns one small child invocation helper: it
starts the known child, waits for terminal exit, relays redirected output and
rejects a nonzero exit before its `finally` cleanup removes that stage.

The same helper serves the startup probe and Product Console smoke.  The
separate timed Product-launch observation remains unchanged because its purpose
is specifically to observe a still-running process during a bounded interval.

The NXVM dependency negative test had correctly rejected its probe but matched
the human-formatted diagnostic names; PowerShell wrapping could split a name.
It now requires the real rejected source paths.  The build-cleanup test still
asserts that a junction is refused wherever Windows permits creation of its
synthetic junction fixture.  A restricted host that cannot create one records
that capability absence rather than reporting a false cleanup-tool failure.

## Verification

- One bounded minimum and one full selected-set probe completed through the
  corrected wrapper on each x64 and x86 build, with the wrapper-owned external
  staging child absent afterwards.
- The staged Product monitor smoke completed on x64 and x86 through the same
  wait path.
- The focused NXVM dependency-negative and repository build-cleanup tests
  pass on x64 and x86.
- Full repository unit suites pass 697/697 on x64 and 697/697 on x86.
- Documentation governance and whitespace checks pass.

The external run provides only bounded lifecycle evidence.  It does not prove
audible sound, interactive gameplay, desktop/RDP input, visual fidelity or
release readiness.  Audio remains a separate unqualified Sound-owner topic.
