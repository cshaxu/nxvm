# M5 T133 S1: Ordered Core Preparation

`core_machine_prepare_executor_cpu`, `_bus`, and `_memory` retain the proven
VM profile order while hiding raw executor-child initialization from VM
composition. VDM can invoke the same core contract when it moves to the
executable machine.

`nxvm-vm-dos-prompt-smoke D:\\home\\repos.hobby\\fdd.img` passed.
The developer artifact is `build/output/nxvm_0_5_0133.exe`
(`4AC38868AEA2EE9B1B642AA567EF8810CDC484C128540A8559B340338EBECFC5`).
