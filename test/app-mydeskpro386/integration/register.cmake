# Original Compaq checks use the deployed product and unchanged external masters.
project_add_console_integration_test(vm-model40-console-ini-smoke)
project_add_ini_integration_test(vm-model40-frame-start-smoke
    compaq-deskpro-386-model-40-1200k.ini)
set_tests_properties(integration.vm-model40-frame-start-smoke PROPERTIES TIMEOUT 210)
project_add_ini_integration_test(vm-ini-cmos-seed-smoke
    compaq-deskpro-386-model-40-1200k.ini)
project_add_ini_integration_test(session-ini-status-smoke
    compaq-deskpro-386-model-40-1200k.ini)
project_add_ini_boot_case(compaq-deskpro-386-model-40-1200k.ini)
