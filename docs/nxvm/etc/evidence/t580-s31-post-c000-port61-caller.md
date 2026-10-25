# M5 T580 S31: Post-C000 port-61 caller boundary

## Question

After the admitted TFT BIOS reaches the post-C000 `F000:EA80h` port-61
service and that service returns, what code actually receives the return?

## Method

The existing `mypc110-real-firmware-post-diagnostic` retains its established
post-C000 route and copies only the first execution point after the service's
local `pop ax; ret` at `EA84h..EA85h`. It does not alter the guest clock,
firmware bytes, port routing or instruction stream. Its 13,000,000-retirement
limit remains containment; the checked result is the real return and caller.

The selected external TFT BIOS is unchanged:

```
232101c88466f311bcc32fbc215a4d7569f695ce19f9c07ca67ce2aee5232312
```

## Observation

Both GCC x64 and x86 runs pass `mypc110.diagnostic/real-firmware-c000-port61-caller`.
Each reports the same semantic result:

```
PC110:POST:c000-port61-caller=f000:0000ea5b bytes=fe cb 75 f6
```

Those bytes are the selected BIOS's `dec bl; jnz EA55h` outer-loop tail. The
previous static `EA38h` routine is therefore related, but it was not valid to
assume its entry from the earlier diagnostic's locally stopped observation.
S31 establishes the caller directly instead.

## Disposition

This selects no new device receiver. The return is back into the already
known finite BIOS delay loop, not an I/O operation. No production source,
Shared/Core contract, ROM, timing shortcut, media input or artifact changed.
The caller checkpoint replaces the weaker `c000-port61-return` CTest entry;
it does not add a duplicate slow diagnostic.

The full finite outer-loop completion remains a possible later semantic
checkpoint only if it is needed to select a source-backed receiver. This result
does not prove POST completion, setup, storage, panel behavior, a guest boot
or a publishable MyPC110 executable.
