if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/vadp.c" source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/vadp.h" header)
file(READ "${PROJECT_SOURCE_DIR}/src/core/CMakeLists.txt" targets)
file(READ "${PROJECT_SOURCE_DIR}/cmake/nxvm/NxvmProduct.cmake" app_targets)

foreach(required "append_ports(routes, 0u, cga_ports"
    "compaq ? compaq_ports : generic_ports"
    "route_count = append_ports(routes, route_count, vga_ports"
    "core_machine_install_port_routes(machine, routes"
    "core_machine_install_port_routes(adapter->machine, routes, route_count)"
    "core_machine_remove_port_routes(adapter->machine, adapter)")
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

if(NOT targets MATCHES "board-base/fdc\\.c board-base/hdc\\.c board-base/vadp\\.c" OR
    NOT targets MATCHES "add_library\\(core-board-base STATIC" OR
    app_targets MATCHES "src/app-[^/]+/.*/vadp\\.c")
    message(FATAL_ERROR "VADP must be built once by the Shared board source list, not App")
endif()

message("VADP-PORT-ROUTES:OK")
