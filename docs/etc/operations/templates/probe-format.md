# Generated DOS Probe Format

Generated probes are the initial compatibility corpus. They are small,
project-authored COM binaries created from reviewed source, so no DOS system
files or third-party applications are required.

## Version 1 Text And Exit Probe

`tools/New-DosProbe.ps1` produces a COM image loaded at `CS:0100h`:

```text
BA 0C 01       mov dx, 010ch
B4 09          mov ah, 09h
CD 21          int 21h
B8 xx 4C       mov ax, 4cxxh
CD 21          int 21h
<ASCII marker> '$'
```

It requires only `INT 21h AH=09h` (write a `$`-terminated marker to standard
output) and `INT 21h AH=4Ch` (terminate with `AL` as exit status). It makes no
BIOS, disk, timer, keyboard, or private DOS-kernel assumptions.

The JSON manifest is part of the test contract: format, origin, interrupts,
marker, exit code, and SHA-256 are explicit. Committed source and manifests are
text; generated `.com` files stay under ignored `build/` or `artifacts/`.
