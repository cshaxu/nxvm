# T482 S3 Console Request Route

`M5:T482:S3:CONSOLE-REQUEST:OK`

Console obtains a copied `vm_product_session_request` from the catalog and
passes that one value synchronously through generic product-session open
options. The manager treats the request as opaque and does not retain it; the
VM provider reads it during open and copies any retained dynamic inputs into
the session. No Console profile branch rebuilds `--profile`, `--cpu`, media,
firmware, or memory arguments.

The request consumer is the sole policy point: default-PC/AT may resolve its
declared CPU/FPU session choices; IBM 5170 and Model 40 remain fixed and reject
CPU/FPU/memory overrides. The Core-machine boundary receives only the resolved
immutable machine plan.

The focused Console route carries a root YAML default-PC/AT `8086`/`8087`
choice and exact memory value as the typed request. The session-provider
regression proves it resolves that pair and rejects a Model-339 CPU override;
the Model-40 and Model-339 Console routes remain green. A production/test sweep
finds no retained VM `--profile`, `--cpu`, `--fpu`, `--memory-kib`, or Model-40
CLI bridge; the one legacy-argument vector is a negative test.
