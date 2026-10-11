# T553 S3 — Four PC App Contract Audit

## Result

No App-owned lifecycle, configuration, composition, failure-cleanup, teardown
or deployment defect was reproduced. No source, test, asset or artifact change
is admitted by S3.

## Single paths verified

All four App entry points provide only a product display name and one fixed
`vm_app_machine_binding`. `core/product/startup.c` is the sole owner of:

- locating and parsing the adjacent `nxvm.ini`;
- validating the copied request and selecting console/window presentation;
- calling the App-owned fixed `prepare` callback;
- creating the private machine, obtaining its Emulator driver, and destroying
  a partial machine on failed construction; and
- formatting the shared PC banner and entering the x86 Product runner.

`core/product/factory.c` is the sole owner of fixed request-to-machine field
copying, `prepare`/create/driver ordering, post-failure destruction, bind,
release, info/speed/floppy extension registration and presentation adaptation.
The four App bindings differ only in machine name, CPU/FDD/BIOS facts, firmware
binding and profile preparation callback. Those are intentional hardware facts,
not lifecycle forks.

## Configuration and deployment audit

The common PC INI parser accepts the approved `display`, `console_control`,
memory and two-slot removable/fixed media grammar. Each App CMake root supplies
its own selected firmware manifest/hash set and exactly one adjacent artifact
root. `verify-product-artifact-roots`, `verify-current-artifact-target`, and
`verify-build-ownership` pass on x64 and x86. The former rejects retired
deployment roots and verifies all four current App roots; the latter two prove
the selected graph builds one truthful current artifact from its App entry.

## Configuration-graph evidence

The eight existing Ninja graphs were reconfigured for My5160, My5170,
MyDeskPro386 and NXVM on both x64 and x86. Each configuration passed every
independent App boundary, every App test-support boundary, and its own selected
XT, AT, Model-40 or default composition graph.

This is a static/contract audit, not a substitute for S4's full dual-width
qualification or manual external-firmware boot validation.
