# SoftPC Shared Corpus Import

Owner: M1 T3. This record identifies the current admitted shared snapshot; it
does not accept the later neutral-contract, lifecycle or product work.
Source commit: `0fb40f482f5aa81464e46e360d0b3f26853ba0ae`.
Root map: `src/lib -> src/lib`; `src/common -> src/common`; `test/lib ->
test/lib`; `test/common -> test/common`. All 180 pinned units are included.

The owner explicitly authorized MIT copying/modification of their owned units on
2026-09-19. Review covers the pinned inventory, notices, source headers, Git
authorship/history and test/build dependencies. All selected units are admitted
under that grant; no independent restrictive notice was found. The grant is not
permission for recovered VM/Compat/MVDM code, firmware, ROMs or unrelated
third-party material; none is copied.

[Per-unit ledger](softpc-shared-units.csv) records each frozen source blob and
destination SHA-256, license basis and disposition. Every current destination
hash equals its blob hash. Source was read from the named Git objects, not the
later-moving SoftPC worktree. The root MIT license applies to these authorized
units; preserve this grant and notices on future transfer.

The four transfer roots are byte-identical to the pin. MyNes-owned root CMake,
presets and standalone verifier compose those roots from the repository's single
root `test/` tree. The refresh removes the old `src/test/` location and accepts
the upstream removal of Common's former x86 debug/xasm units. Optional receiver
components and downstream product integration remain separately admitted work.
