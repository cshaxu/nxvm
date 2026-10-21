# T7 S57 — COP Command-Admission Classification

## Decision

The command-role terminal selected by S56 can arise at exactly three existing
command-admission branches: a prior pending operation, an invalid command
request shape, or an unsupported local command classification.  This S adds a
resettable safe enum to the existing COP provider-failure observation and sets
it immediately before those unchanged returns.

The change neither admits a new command nor changes a return value.  The
existing detailed COP failure record remains intact; the new aggregate carries
no command value, address, input byte, instruction or execution position.

## Verification

The Board smoke exercises all three existing rejection branches through the
installed X86 Core route and proves their distinct safe categories, role,
write direction and status.  The complete repository-only MyRaidenII unit set
passed on x64 and x86 (11 executables per width).

One bounded registered owner-ROM wrapper run per width reports the same safe
`unsupported` command-admission category.  The wrapper-created staging child
was empty after each run.  No raw probe output, command word, address, trace,
ROM/media data or external implementation source is retained.

## Transfer

The selected receiver is the local unsupported-command family.  It requires a
complete source-policy-compliant command-family inventory and a separately
admitted behavior contract; it may not make unsupported commands successful
by default, infer a command value from external media, or change the pending
and request-shape rejection rules.
