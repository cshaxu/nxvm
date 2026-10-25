# M5 T580 S42 — MyPC110 runner-path containment

## Observation

The embedded-TFT preview was copied with its unchanged adjacent `mypc110.ini`
and launched hidden through a bounded command pipe. The native cooked-console
reader did not create a monitor transaction from redirected standard input; the
wrapper ended with no guest output or durable child process.

## Disposition

This is a host-console harness limitation, not evidence about BIOS progress or
the Turbo runner. Extending it, injecting input, or observing execution from a
separate path would test a different Product route. The four task-local files
were removed through `Clear-Build.ps1` after the copied INI hash matched the
owner asset.

No source, firmware, Core, Shared component, product artifact or receiver was
changed. T580 remains source-gated.
