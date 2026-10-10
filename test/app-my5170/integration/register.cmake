# Original IBM AT checks use its real deployed EXE and unchanged external inputs.
# The boot/console observers remain single family integration harnesses.
project_add_console_integration_test(vm-app-console-lifecycle-smoke)
project_add_ini_integration_test(vm-ini-cmos-seed-smoke
    ibm-5170-model-339-1200k.ini)
project_add_ini_boot_case(ibm-5170-model-339-1200k.ini)
