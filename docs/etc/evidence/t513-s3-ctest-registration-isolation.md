# T513 S3: CTest Registration And Isolation

The CMake registry declares exactly 20 named `integration` cases, one for
each S1 row.  CMake supplies only the row ID and owner-provided cache inputs;
the S2 runner remains the sole table, configuration and terminal-policy
owner.  No image, firmware path, checksum or guest-media byte is tracked.

Every case has its own build-tree working directory, a 70-second timeout and
`SKIP_RETURN_CODE 77`.  Empty cache inputs therefore report unavailable owner
assets rather than a false pass or failure.  The inputs are read-only and the
runner has no shared writable file, port or host-input dependency, so the 20
cases have no blanket serial or resource-lock restriction.

The existing T344 registration gate now checks this multi-case runner as an
exact 20-entry exception, removes those entries, then retains its prior
one-target/one-route count for every canonical target.  It also accounts for
the pre-existing 8088 timing-results auxiliary route, which was intentional
but omitted from the auxiliary manifest.

Focused proof on the empty-cache configuration:

- `verify-t344-unit-registration` reports
  `M5:T500:S4:TEST-NO-DUAL-REGISTRATION:OK:335`;
- the matrix CTest selection reports 20 skipped, zero failed, in 0.49 seconds.

External execution and any resulting product repair remain S4 work.
