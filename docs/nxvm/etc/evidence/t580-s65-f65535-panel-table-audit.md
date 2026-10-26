# M5 T580 S65 — F65535 panel-table audit

## Question

The reported ERROR/Easy-Setup blue-versus-pink appearance could not justify an
RGB/BGR swap by itself. This audit asked whether the selected embedded TFT
BIOS writes a C&T F65535 extended-register (XR) panel table through `3D6h`
and `3D7h`, and whether the existing MyPC110 route loses such writes.

## Evidence and method

`Open-Source-PC110/Discovery/65535/readme.md` is L2 research only. It records
that real PC110 panel configuration is represented by F65535 XR registers and
names live panel values, but it is neither imported code nor a claim about the
selected BIOS image.

The existing real-firmware harness was temporarily extended to record the
existing `3D6h` index and `3D7h` data writes. It was run on the normal,
unchanged embedded TFT BIOS through its retained-power, POST and `INT 19h`
route:

```text
PC110:BOOT:int19=1 target=f000:0000e6f2
PC110:POST:xr-writes=0 capacity=64
PC110:POWER:button=1 restore=1 rsm=1 status=0 pc=000fe6f2 resets=4 executed=64883326
```

The earlier C000-option-ROM bounded replay likewise reached and left the
option ROM (`c000=0/1714`) with zero retained port records. The full route
therefore improves the evidence boundary: this BIOS reaches its ordinary
bootstrap handoff without using the observable XR programming path. The
temporary harness change was removed after the observation. No unregistered
diagnostic mode or test-only production entry remains.

The existing registered x64 real-firmware FDC receiver still passes after
rebuilding the restored harness:

```text
mypc110.integration/real-firmware-fdc-reset ... Passed (33.83 s)
```

## Disposition

There is no observed F65535 XR write for the current TFT BIOS route, hence no
evidence that the controller's XR storage, `3D6h/3D7h` board adapter, or a
panel-table interpretation changes the reported colour. Standard VGA
Attribute/DAC/PEL colour remains the existing owner; S57's real-frame capture
already records the programmed `BE9E96` rose background on this path.

No production change is justified. This result does **not** prove an RDP/DPI
issue, nor does it qualify physical LCD/TFT/DSTN scanout. Those remain outside
this bounded audit and require fresh visual evidence if they are admitted
later.
