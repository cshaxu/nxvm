file(READ "${SOURCE_ROOT}/src/app-mynes/core/snapshot.c" source)
# Byte arrays remain valid bulk operations. A pointer to a scalar must not
# serialize neighbouring struct members or depend on their layout.
if(source MATCHES "(read|write|r|w)\\([^\n]*, &[a-zA-Z_][a-zA-Z0-9_>.\\-]*, ([2-9]|[1-9][0-9]+)u\\)")
    message(FATAL_ERROR "Snapshot scalar span depends on adjacent fields")
endif()
message(STATUS "Snapshot scalar fields are explicit")
