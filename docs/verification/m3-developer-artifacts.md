# M3 Developer Artifact

M3 retains one final task-level local artifact rather than subtask smoke
executables. It is a GCC build of the retained whole-PC NXVM path, so it starts
the NXVM Console and can configure and boot developer-provided disk images.
It is not the future `ntvdm64 run` product and bundles no guest media.

| Task | Local artifact | Source commit | SHA-256 |
| --- | --- | --- | --- |
| T5 | `build/output/nxvm-m3_t5.exe` | `e1dd8bedc177be2cace930eac8dfc0a8da6bac0b` | `8776d9d577392fa2792fdeeca9b52bea718cf15cabf490182dee5fc771a32c8d` |

- Build preset: `m3-nxvm-task-artifacts-gcc`.
- Runtime banner: `Neko's x86 Virtual Machine [0.4.015d.m3t5]`, followed by
  `Copyright (c) 2012-2014 Neko.`
- Console verification: piping `exit` reached `Console>` and returned exit
  status 0.
- The superseded `ntvdm64-m3_t*_s*.exe` smoke copies were deleted from the
  ignored local output directory. Smoke targets remain build-tree verification
  tools only.
