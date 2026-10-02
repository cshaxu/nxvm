if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/vadp.c" source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/vadp.h" header)
file(READ "${PROJECT_SOURCE_DIR}/cmake/nxvm/NxvmProduct.cmake" targets)

foreach(required "append_ports(routes, 0u, cga_ports"
    "compaq ? compaq_ports : generic_ports"
    "route_count = append_ports(routes, route_count, vga_ports"
    "core_machine_install_port_routes(machine, routes"
    "core_machine_install_port_routes(adapter->machine, routes, route_count)"
    "core_machine_remove_port_routes(adapter->machine, adapter)"
    "core_machine_memory_unregister_owner(memory, candidate)")
    string(FIND "${source}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "VADP Core-owned route boundary is missing ${required}")
    endif()
endforeach()

foreach(forbidden "t_port" "core_machine_port_add_"
    "core_machine_port_registration_begin" "core_machine_port_rollback_registration"
    "core_machine_port_unregister_owner" "adapter->port")
    string(FIND "${source}${header}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "VADP retains raw port access: ${forbidden}")
    endif()
endforeach()

string(FIND "${targets}" "    src/app-nxvm/devices/vadp.c\n)" runtime_position)
if(runtime_position LESS 0)
    message(FATAL_ERROR "VADP is not owned by the Core runtime target")
endif()

message("M5:T540:S15:VADP-PORT-ROUTES:OK")
