# T496 S3 XT BYOB Run-Budget Diagnostic

`M5:T496:S3:XT-BYOB-PROBE=HOST-WRAPPER-INVALID`

The owner-authorized external IBM XT firmware/media probe was started in Turbo
mode after S2. The first host wrapper did not wait for the child process, so it
could not establish a guest result. A bounded stage capture then proved that
the Core call returns repeatedly; the process was explicitly terminated after
that host-side observation. No media, firmware, path, hash, output text or
screenshot was retained.

The initial host-wrapper observation therefore transfers no keyboard, PPI,
firmware, DOS, CPU-execution or run-budget conclusion. The next S must use one
host-waited process with an explicit wall deadline and report only the probe's
semantic result.
