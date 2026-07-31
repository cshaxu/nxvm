# M5 T1 Task Artifact Verification

- Source build: M5 T1 implementation worktree, before its closure commit.
- Artifact: ignored local `build/output/nxvm-m5_t1.exe`.
- SHA-256: `07786eba9fed96433e88caf1a4b384f4835c344425170b4c760b8c872da0100d`.
- Runtime identity:
  `Neko's x86 Virtual Machine [0.4.015d.m5t1]` and the retained 2012-2014
  copyright line.
- Smoke: piping `exit` reached the retained NXVM Console and returned status
  zero. No guest media was supplied.

The task artifact is a developer artifact only; it is not a release, does not
bundle protected media, and retains the whole-machine Console path.
