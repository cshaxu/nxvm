# T440 S1: Model-40 Immutable Configuration

## Boundary

The VM session owns the selected Model-40 configuration. Its two construction
routes now share one private fixed-configuration initializer, which fixes the
existing 1 MiB, 80386, D4 and timing contract once. The public production BYOB
route rejects an explicit non-1-MiB memory request. The existing runtime memory
reconfiguration entry rejects a materialized Model-40 before invoking Core.

Core remains machine-neutral and retains the same reconfiguration behavior for
legal generic sessions. No profile state, configuration framework or Core
machine-name branch was added.

## Route Sweep And Proof

The factory parser already rejects generic overrides for non-default profiles.
The public BYOB constructor now rejects its otherwise ignored incompatible
memory input. The test-only private constructor has no caller-supplied memory
input and receives the same private fixed initializer. The sole runtime session
memory mutation API now rejects Model-40 before its Core handoff; direct Core
tests remain Core-only and are not a VM route.

`vm-model40-byob-s20-smoke` uses the production BYOB route and proves both
creation-time rejection and runtime rejection. It checks that Core memory,
session fixed configuration and retained request state remain unchanged. Output
includes:

```
M5:T440:S1:MODEL40-IMMUTABLE-CONFIGURATION:OK
```

`vm-session-reconfigure-smoke` proves the existing generic session route still
successfully changes memory. The Model-40 private-composition regression also
passes.

## Artifact

The developer artifact is `vm-0-5-0440`; copied artifact
`build/output/nxvm_0_5_0440.exe` has SHA-256
`E0A144BC0DBF50F10DB65D718F02485C29BBCA72694F3CA246ACEEC592C69D1F`.

The counted tracked source/test change is 41 added and 61 removed lines, net
-20. The shared private initializer removes one entire duplicated Model-40
configuration literal; the retained additions are the two boundary checks and
their state-preservation proof. Documentation governance and the fast current
smoke gate pass. The full gate reaches the pre-existing T344 historical fixture
count assertion (71 expected, 75 found); this task changes no fixture
constructors.
