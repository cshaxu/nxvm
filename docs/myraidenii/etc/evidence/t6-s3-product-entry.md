# M1 T6 S3 — Product Entry Evidence

## Scope

S3 adds the sole MyRaidenII process entry and build target.  It does not
prepare, open, copy, hash, embed or publish protected game input.

## Route

`main` performs exactly two product operations:

1. `myraidenii_product_config_load(argc, argv, &config)` resolves the already
   established adjacent lowercase `myraidenii.ini` policy.
2. `myraidenii_product_run(&config)` enters the existing S1 composition owner.

The entry does not parse configuration independently, construct Board/Machine
objects, own a Session/UI, or provide a fallback input route.

## Build Identity

The local target is `myraidenii-0-0-0006`; its build-tree output name is
`myraidenii_0_0_0006`.  There is deliberately no post-build Asset deployment
or generated INI.  A future release packet must provide and validate the
external individual-ROM configuration rather than creating a misleading
runnable binary without its owner-supplied inputs.

## Verification

- `cmake --build build/.ninja/gcc-x64-release --target myraidenii-0-0-0006 --parallel 8`: passed.
- `cmake --build build/.ninja/gcc-x86-release --target myraidenii-0-0-0006 --parallel 8`: passed.
- Complete `ctest -L myraidenii` selection: 12/12 passed on x64 and 12/12 passed on x86.
- No desktop, RDP, external-ROM, executable deployment or gameplay verification was performed or claimed.
