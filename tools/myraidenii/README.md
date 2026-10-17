# MyRaidenII Tools

`Verify-RomInventory.ps1` audits an exact external ZIP against reviewed metadata:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/myraidenii/Verify-RomInventory.ps1 `
  -Inventory docs/myraidenii/etc/evidence/t1-s1-rom-inventory.json `
  -Archive <owner-supplied-archive>
```

This is an archival verifier, not the future runtime ROM loader. It verifies
the recorded container length/SHA-256 and every expected member's name, length,
SHA-1 and SHA-256. The exact container fingerprint deliberately rejects a
repacked ZIP even if its payloads are equivalent; it proves reproducibility of
the audited input, not compatibility of every possible package.

Member reads are bounded to the reviewed length and the selected board's
2 MiB image bound; the selected-set envelope is at most 15 images and a 32 MiB
container. Duplicate/case-colliding, path-like, missing or altered
members fail. No extraction, download, payload output or source modification
occurs. The one read-only archive handle is retained throughout verification.
Only structured metadata is printed on success; errors terminate the command.

ROM-free synthetic unit coverage lives in test/myraidenii/unit/tools. Actual
owner-ROM execution will belong to integration, not this metadata check.
