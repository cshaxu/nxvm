# M5 Toolchain And Build-Tree Hygiene

## Purpose

Restore one reproducible default MinGW toolchain configuration, remove
classified obsolete build trees, and reduce current build instructions to their
operational contract.

## Boundary

No machine-local compiler path, binary, source code, product behavior or build
target is introduced. The default preset remains PATH-based and a fresh tree
must select compiler, archiver and ranlib from one toolchain family.

## Planned Subtasks

1. **S1 - Fresh default toolchain baseline.** Record the mixed cached tools,
   rebuild the default configuration from a clean tree, and prove one compiler,
   archiver and ranlib family plus current artifact/gates.
2. **S2 - Temporary-tree classification and cleanup.** Freeze the exact
   non-default build-tree ledger, preserve only `build/output` and the fresh
   default tree, then remove every verified obsolete task/experiment tree.
3. **S3 - Operational-document convergence.** Move historical M0/M1/MSVC
   narrative out of the current toolchain operation path, retaining one concise
   default command, gate command and opt-in ccache command.

## Stop Condition

Stop for a required machine-local configuration commitment, a toolchain output
identity change, or an unclassifiable build tree that may contain needed user
data.
