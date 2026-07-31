# M5 T7 S3 Console Compatibility Verification

The `m5-t7-nxvm-artifact-gcc` target now builds the retained original NXVM
full-PC Console target. It emits:

```text
Neko's x86 Virtual Machine [0.4.015d.m5t7]
Copyright (c) 2012-2014 Neko.
```

Scripted `help`, `info`, and `exit` retained the original command list and
device information. A delayed interactive-pipe check sent `debug` with no
mounted image, observed `Console> -`, sent `q`, observed the returned
`Console>`, then sent `exit`; the process returned zero. Delaying `q` until
after debugger entry is necessary because the retained debugger flushes stdin
before its prompt.

The ignored local task artifact is `build/output/nxvm-m5_t7.exe`, SHA-256
`f5a8f5fd0e7559b3e0ffa71d22ea60c8cebc49c015f04477241466e9bd86c923`.
It is a developer artifact only and contains no guest media. This result
restores compatibility but does not close M5: its actual full-PC execution
remains baseline-backed until the migration work is complete.
