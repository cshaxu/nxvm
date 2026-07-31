# M4 T6 S1 CPU Verification Design Review

- Reviewed the current invalid-opcode path and partial i386 dispatch markers in
  the retained NXVM CPU source. They do not establish complete i386 semantics.
- Reviewed the read-only historical Bochx design: it bridges Bochs CPU/memory,
  executes NXVM before/after a reference instruction, compares state, records
  linear memory access, and stops on divergence.
- The design records the local MS-DOS 6.22 `MEM` #UD observation without
  committing or hashing protected media, and requires a bounded capture plus a
  project-owned minimal probe before repair.
- M5 now begins with CPU capability/microprobe and optional Bochx experiment
  work, while prohibiting Bochs material and reference dependencies in default
  builds, tests, artifacts, and releases.
