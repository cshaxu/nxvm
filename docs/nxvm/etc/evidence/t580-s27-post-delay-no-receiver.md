# M5 T580 S27 — post-delay receiver disposition

## Question

After the source-qualified C000 path returns through the finite port-61 delay
at `F000:EA86..EA92`, does the real TFT BIOS make a next I/O access that can
select one new App-local receiver?

## Observation

The established x64 real-firmware diagnostic reaches the delay return at
`F000:EA84`.  A temporary, uncommitted trace widened its existing I/O filter
after that return and continued to the existing 13,000,000-instruction
containment ceiling.  It observed no subsequent I/O access:

```
status=0 reason=1 pc=000fea8a executed=13000069 resets=1
c000=0/828865 port61-wait=1/1
```

The temporary diagnostic path was removed.  The instruction ceiling is not a
success condition and no CTest or production route was retained from it.

## Disposition

The FDC37C665IR at `3F0h..3F7h` is hardware-confirmed by the external PC110
research, but no access to it occurs along this reached BIOS path.  Likewise,
the documented `F9h/FBh` EC/ED configuration gate cannot yet be assigned a
guest-visible closed-state contract from this trace.  Neither receiver is
implemented in S27.

This leaves the existing source-backed EC/ED, EC-block and EC-mailbox receivers
unchanged.  It does not qualify POST, Setup, DOS, panel, power/MCU or release
behavior.
