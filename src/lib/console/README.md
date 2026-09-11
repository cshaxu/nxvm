# console

`console` provides the platform-neutral `lib_console` logical Console object,
copied input/output values, and lifetime/output contracts. It depends only on
`types`. `host` binds one caller-owned logical Console to native I/O;
`ui-console` creates one for its raw Console lifecycle. This component contains
no native handle, platform mode, monitor, or application lifecycle policy.
