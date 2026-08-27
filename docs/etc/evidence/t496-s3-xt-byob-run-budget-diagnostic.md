# T496 S3 XT BYOB Run-Budget Diagnostic

`M5:T496:S3:XT-BYOB-PROBE=RUN-NONRETURN`

The owner-authorized external IBM XT firmware/media probe was started in Turbo
mode after S2. It did not return from its first `core_machine_run` call, so its
own 15-second no-progress classifier and its semantic DOS checkpoint checks
could not run. The externally started probe process was explicitly terminated
after observation; no media, firmware, path, hash, output text or screenshot
was retained.

The existing Core public run budget counts only completed retirements and
optional guest ticks. The initial diagnostic therefore transfers no keyboard,
PPI, firmware or DOS conclusion. It identifies the earliest owner to audit as
the Core CPU-execution/run boundary: a finite host-control quantum must return
even while the current guest instruction has not yet completed. The next S
must freeze the complete long-running instruction/repeat/blocked-retirement
batch and prove its existing behavior before selecting any repair.
