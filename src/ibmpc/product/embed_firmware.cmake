# Build-only byte conversion: no raw firmware or input paths enter tracked source.
set(source "#include \"ibmpc/machine/input_interface.h\"\n\n")
foreach(role IN ITEMS BIOS_0 BIOS_1 VIDEO CMOS FONT)
    set(path "${INPUT_${role}}")
    if(path STREQUAL "LIB_NULL")
        set(view_${role} "{LIB_NULL, 0u}")
        continue()
    endif()
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Missing firmware build input: ${role}")
    endif()
    file(READ "${path}" bytes HEX)
    if(bytes STREQUAL "")
        message(FATAL_ERROR "Empty firmware build input: ${role}")
    endif()
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${bytes}")
    string(REPEAT "0x[0-9a-f][0-9a-f]," 16 line_pattern)
    string(REGEX REPLACE "(${line_pattern})" "\\1\n" bytes "${bytes}")
    string(APPEND source "static const lib_u8 firmware_${role}[] = {\n${bytes}\n};\n\n")
    set(view_${role} "{firmware_${role}, sizeof(firmware_${role})}")
endforeach()
string(APPEND source "const vm_machine_assets vm_app_firmware = {\n"
    "    .bios = {${view_BIOS_0}, ${view_BIOS_1}},\n"
    "    .video = ${view_VIDEO},\n"
    "    .cmos_seed = ${view_CMOS},\n"
    "    .font = ${view_FONT}\n};\n")
file(WRITE "${OUTPUT}" "${source}")
