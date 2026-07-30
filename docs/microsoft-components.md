# Optional Microsoft Guest Components

## Status

This is an experimental, post-default-backend compatibility path. It must never
block the project-owned DOS backend or the normal release.

## BYOB Contract

Users may import legally obtained local component sets, initially investigated
as `NTIO.SYS`, `NTDOS.SYS`, and `COMMAND.COM`. The import command records a
profile identity, file size, SHA-256, and private local path. It neither copies
the files into the repository nor uploads or redistributes them.

Profile validation rejects incomplete sets and arbitrary mixing of Windows
versions. The first research profile, if approved, is XP SP3 x86. No component
is accepted merely because its filename matches.

## Research Scope

The future adapter may implement a guest binary loader, BOP trap/dispatcher,
startup trace, and minimal host services. The staged evidence targets are:

1. load a validated component and capture its first BOP;
2. observe NTDOS/NTIO initialization and `COMMAND.COM` startup;
3. demonstrate prompt, `DIR`, `TYPE`, and `EXIT`.

These are research milestones, not compatibility promises. OpenNT and NTVDMx64
may guide questions, but tests and independently maintained adapter code decide
released behavior.
