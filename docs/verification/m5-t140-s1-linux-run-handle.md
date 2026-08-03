# M5 T140 S1: Linux Run-Handle Parity

## Result

`lnxcStartMachine` and detached pthread creation are removed. Linux Console
now allocates a private backend record through the session-owned
`vm_platform_run_handle`, starts joinable kernel/display pthreads, and reports
only execution/display events from those workers. The outer lifecycle requests
stop, joins both started workers, then finalizes curses and releases the
terminal lease.

The allocation-failure path releases the lease before any worker exists. A
display-worker creation failure uses request-stop, join, and finalize for the
already-started kernel. No Linux worker calls `lnxcdispFinal()` or releases the
lease.

## Static Verification

The following source scans completed on the Windows development host:

```text
rg "lnxcStartMachine|PTHREAD_CREATE_DETACHED|pthread_attr|pthread_detach|ThreadDisplay|ThreadKernel" src/vm/platform/linux
rg "lnxcdispFinal|core_platform_host_surface_lease_release|pthread_join|pthread_create" src/vm/platform/linux/linuxcon.c
```

The first scan has no matches. The second shows `pthread_create` only in
run-handle start, `pthread_join` only in run-handle join, and curses/lease
teardown only in run-handle finalize, except the pre-worker allocation failure
lease release.

The Windows GCC T139 regressions also remained green:

```text
M5:T139:S1:RUN-HANDLE:OK
M5:T70:S2:DOS-PROMPT:OK
```

## POSIX Handoff

Native POSIX verification is intentionally outstanding. The POSIX maintainer
must configure Linux GCC and run Console normal stop, failed display-worker
creation, terminal-lease contention, and FDD DOS-prompt checks before making a
platform-runtime compatibility claim. This task changes no Windows runnable
path and produces no Windows artifact.
