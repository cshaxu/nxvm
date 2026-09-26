# T538 S3: MyNES Console Receiver

NXVM-hosted M5 T538 S3 explicitly includes MyNES as a receiving App for Shared
6a3f3cb25. MyNES T43 stays closed; no new MyNES task, source or configuration
change is implied. Console backing storage no longer requires the visible
viewport to contain every guest cell. No font scaling or clipping is added.

Both optimized stripped 0043 product targets were rebuilt and deployed under
assets/mynes. Full suites pass 132/132 on x64 and x86: 127 non-desktop and five
serial desktop tests, including native Console and Window. All six manifests
pass. Snapshot, INI and media are unchanged.

| Artifact | SHA-256 |
| --- | --- |
| mynes_0_0_0043_x64.exe | 6F368B9DC933F73302A1F0B5CD36623A3A61DA241B4D9C643BCCE26E7F668AE4 |
| mynes_0_0_0043_x86.exe | 8BA51B5F1A52BDC0FBE86D46B45EB6C412FEE953610CC4F6765C7948BE98DAFD |

This receiver is delivered as its own MyNES-targeted P under M5 T538 S3;
Shared implementation and NXVM artifacts belong to separate commits.
