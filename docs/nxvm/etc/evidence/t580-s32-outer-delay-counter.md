# M5 T580 S32: PC110 outer-delay counter fact

## Question

After S31 establishes that the real TFT BIOS returns from `F000:EA80h` to
`F000:EA5Bh`, is a complete `EA38h` outer-delay-loop diagnostic a bounded,
useful next checkpoint?

## Method

The existing `c000-port61-caller` diagnostic remains the sole checkpoint.  At
the first real caller it copies the low byte of the already delivered
retirement CPU snapshot's `EBX`, then prints it alongside the existing caller
point.  It changes no guest state, device route, timing source or instruction
stream.

The selected TFT BIOS decodes as:

```text
EA55  mov cx,040Bh
EA58  call EA80h
EA5B  dec bl
EA5D  jnz EA55h
...
EA7F  ret
```

## Observation

Both established host widths passed the strengthened existing diagnostic:

```text
PC110:POST:c000-port61-caller=f000:0000ea5b bl=6f bytes=fe cb 75 f6
```

The point is after `dec bl`, so the immediately preceding value was `70h`.
The first observed service already consumes one of 112 loop iterations; roughly
111 same-shape waits remain before `EA7Fh` can return.  The observed one-call
checkpoint takes 34.69 seconds on x64 and 43.09 seconds on x86.  A registered
complete-loop test would therefore be roughly hour-scale while only proving a
repetition of an already-owned port-61 wait.

## Disposition

Do not register a complete outer-loop diagnostic.  It would add no receiver,
no source-backed device contract and no guest qualification fact, while
violating the repository's requirement to avoid redundant slow diagnostics.
The strengthened existing caller test is retained because it captures the
semantic counter fact that supports this disposition.

This does not prove POST completion, setup, storage, panel behavior or guest
boot.  It neither selects a new implementation receiver nor changes
production source.

## Suite-budget correction

The required complete unit run exposed three static fixture/configuration
checks whose inherited 30-second CTest limit was lower than their ordinary
measured run time: Lib boundary self-test, Lib Types-boundary self-test and
Emulator verifier-negative.  Each passes when run normally; the Emulator
check alone took 32.14 seconds without parallel contention.  Their existing
CTest overrides now use the approved 300-second limit.  No assertion,
fixture, production source or component contract changed.  The full unit
suite then passed 667/667 on each of x64 and x86.
