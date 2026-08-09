# M5 T308: Exception-Delivery Family

## Pending Acceptance

This is closure-preparation evidence for coordinator review. T308 remains
active in `STATUS.md`; this record does not close the task or advance the
Queue.

## Scope And Semantics

T308 completed the admitted 80386 protected 32-bit error-delivery surface in
the single core executor. Existing `#GP`, `#NP`, `#SS`, and `#TS` producers
reuse the existing `ExecFinal` to `_e_except_n` to same- or outer-CPL planner
route, preserving their normalized error code in the frame below saved
instruction state. No second dispatcher, recovery engine, stack route,
selector/cache owner, public ABI, or product UX path was added.

The S6 correction applies Intel 80386 PRM Chapter 9 contributor-pair rules:
an existing `#TS`, `#NP`, `#SS`, or `#GP` delivery attempt which itself reaches
another contributory preflight fault becomes `#DF(0000)` through the existing
vector-8 route. A failed vector-8 delivery leaves the machine state restored
and records a bounded terminal `#DF(0000)` diagnostic. This is not a claim of
an implemented triple-fault shutdown/reset policy. Read-only comparison used
the versioned paths recorded in the [T308 evidence](../etc/evidence/t308-exception-delivery-admission.md):
Intel 80386 PRM Chapter 9, Bochs 2.6 `cpu/exception.cc` and
`cpu/fetchdecode.cc`, and PCjs 2.00.0 `cpux86.js` and `x86help.js`. No source
was copied.

## Verification And Observation

- Current artifact: `build/output/nxvm_0_5_0308.exe`.
- SHA-256:
  `A6BE95BB5C9647F72DB8C28982C8DEFB95CBDA9C9FE92F04F3B4EDC592BC646F`.
- `current-gates-gcc` passed 51 static/governance targets and 137/137 CTests.
- Direct focused and retained execution preserved the T308 S2/S3/S5/S6
  markers plus T301, T304, T305, T306, T307, T261, T302, and T303 coverage.
- Documentation governance, current artifact verification, and
  `git diff --check` passed.

The closure observation launched the artifact hidden for 45 seconds with no
media and no guest input. The process remained alive, but exposed no host
window handle or title. It was then stopped by the observer. This is a
host-automation limitation only: no guest checkpoint, product progress, or
regression conclusion was obtained.

## Deferred Boundaries

T308 does not admit page-fault combinations, hardware or NMI delivery,
task/virtual-8086 paths, or actual triple-fault shutdown/reset. Those require
separate future admission and do not change the completed same/outer error
frame or bounded contributory containment contracts.
