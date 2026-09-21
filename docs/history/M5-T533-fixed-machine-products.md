# M5 T533 Fixed-Machine Products

Owner admitted this task on 2026-09-21 to implement the approved fixed-profile
product architecture while retaining all implemented XT, AT, DeskPro 386 and
default PC/AT machines. Each executable will select exactly one Profile at
build time and resolve its lawful firmware solely from the build-provided
external BYOB asset root. The retained candidate proposal is
[Fixed-machine products and architecture consolidation](../proposals/m5-fixed-machine-products.md).

## S1: Baseline And Contract Ledger

S1 inventories every existing machine constructor, reset/provider branch,
asset role, configuration key, build target and owned test before relocation.
It freezes the direct profile/build/INI contract and maps each retained machine
and CPU variant to its next migration receiver. S1 changes no production
behavior; its ledger is the prerequisite for later source moves.
