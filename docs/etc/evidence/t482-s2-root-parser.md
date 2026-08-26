# T482 S2 Root Parser

`M5:T482:S2:ROOT-PARSER:OK`

`session_catalog.c` remains the single parser owner. It now accepts only
`schema: nxvm-session` plus root `profile`, `memory_kib`, `cpu`, `fpu`,
`display` and `boot`; session media and BYOB resource blocks retain their
existing session resource structure. The legacy `nxvm-session/v1` and nested
`machine.profile` forms are rejected. CPU/FPU are syntactically preserved for
the request consumer: only a profile that explicitly declares them as session
choices may accept them.

The catalog stores `vm_product_session_request` values and returns a copied
request; it neither exposes catalog storage nor returns YAML-backed pointers.

The parser owns syntax only. Profile selection, permitted memory/media/boot,
Model-40 BYOB, display policy, and default-PC/AT CPU/FPU choice policy are
validated by the copied-request consumer; the catalog contains no
profile-specific policy. The Console supplies that one request directly and no
longer re-encodes it as profile CLI arguments.

The focused `vm-product-session-catalog-smoke` passes with root acceptance,
v1/nested rejection, and CPU/FPU syntax preservation without catalog policy.

`M5:T482:S2:REJECTIONS:OK`

`M5:T482:S2:IMMUTABLE-REQUEST:OK`

The companion request-consumer regression proves a `default-pc-at` 8086/8087
choice reaches the existing resolver, while an IBM 5170 CPU override is
rejected. Thus a CPU/FPU string in the parser result is not machine authoring
and does not create a second configuration path.
