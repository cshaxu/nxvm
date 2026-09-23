# M5 T34 S5 - Release Manifest Evidence

`tools/Generate-ReleaseManifest.ps1` is the single generator and verifier for
the versioned release set. It covers only `mynes_0_1_0011_x64.exe`,
`mynes_0_1_0011_x86.exe` and the sole editable `mynes.ini`; comparison binaries,
ROMs and save files are intentionally absent.

The checked manifest records product and version, the frozen source revision,
CMake version, both target/compiler identities and SHA-256 for every declared
artifact. It contains no owner-local ROM path or ROM byte. Verification derives
the expected artifact lines from the frozen revision already recorded in the
manifest, so later commits to the generator or evidence do not invalidate a
previous release record.

After no-op x64/x86 product rebuilds, two consecutive generation passes gave
the same manifest SHA-256:
`E731E6B47E258E6619E8AE43A9265C252C09FB62063AC62C34BB49F4C7EF9F38`.
The generator's verification mode then passed against the checked artifacts.
