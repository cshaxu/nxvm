# T580 S79 — Derived DSTN guest qualification

S78 established power/resume and boot handoff with the separately embedded
derived DSTN firmware on x86 and x64. It did not establish Setup, DOS, pointer
input or a usable desktop for this variant. The derived firmware is not
represented as an independently sourced pristine original dump.

S79 reuses the same real-firmware diagnostic, unchanged guest execution,
task-owned disk overlays and Setup oracle, linked to the DSTN firmware target
instead of TFT. The both-width guest runs are in progress. The current
oracle was previously verified for TFT; any mismatch must be investigated
as a possible variant-specific distinction rather than overridden or counted
as failure of the product without further evidence.

A read-only comparison of the two already reviewed 262,144-byte external
BIOS inputs found 18 differing bytes, all between offsets `201CBh` and
`20B04h`. This is consistent with the recorded panel-patch provenance; it
does not prove that their decoded Setup images or guest behavior are equal.
The firmware bytes themselves remain outside the repository.

Both-width live DSTN runs reached the actual `5000:0000` Setup entry; the
232,185-byte loaded Setup image matched the existing reviewed oracle in each
run. Guest startup and input verification are still running. This demonstrates
image identity only; the eventual visible DSTN panel must be checked separately.

The x64 DSTN diagnostic has completed successfully: the original guest
passed Setup and DOS loader checkpoints, accepted four calibration points
and an independent PS/2 relative input, and produced the Personaware launcher
without a Core fault. Its final frame SHA-256 is
`6149FFD194A8B24F4C772C719AB3793E40FE6E9AC2829BA2A55C32E859004A62`,
the same as the corresponding TFT guest. This is guest-frame evidence; no
physical DSTN panel or desktop/RDP presentation was observed. The x86 run
also completed successfully with the same final-frame hash, no Core fault,
all four calibration points and the isolated PS/2 input accepted. Both runs
ended at their prescribed instruction budget, reporting status zero and
164,836 delivered exceptions; those OS-era VM86 deliveries are not treated
as faults. The x86 and x64 diagnostic exit codes are both zero.

Release-readiness inspection confirms that `src/app-mypc110/CMakeLists.txt`
currently creates only the fixed-firmware preview targets. The existing
`verify_product_artifact_roots.cmake` does not yet include `assets/mypc110`.
After guest qualification, a subsequent bounded S must add the four named
release targets and owner-specific artifact verification while preserving the
adjacent INI and the two distinct firmware bindings. Copying a preview by
hand is not a substitute for that build/deploy contract.
