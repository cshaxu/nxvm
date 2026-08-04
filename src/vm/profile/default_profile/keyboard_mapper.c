#include "type.h"

#include "vm/profile/default_profile/keyboard_mapper.h"

static uint8_t vm_profile_default_keyboard_map_ascii(uint16_t value)
{
    static const uint8_t scan_codes[128] = {
        [0x08] = 0x0eu, [0x09] = 0x0fu, [0x0d] = 0x1cu,
        [0x1b] = 0x01u, [0x20] = 0x39u,
        ['0'] = 0x0bu, ['1'] = 0x02u, ['2'] = 0x03u, ['3'] = 0x04u,
        ['4'] = 0x05u, ['5'] = 0x06u, ['6'] = 0x07u, ['7'] = 0x08u,
        ['8'] = 0x09u, ['9'] = 0x0au,
        ['a'] = 0x1eu, ['b'] = 0x30u, ['c'] = 0x2eu, ['d'] = 0x20u,
        ['e'] = 0x12u, ['f'] = 0x21u, ['g'] = 0x22u, ['h'] = 0x23u,
        ['i'] = 0x17u, ['j'] = 0x24u, ['k'] = 0x25u, ['l'] = 0x26u,
        ['m'] = 0x32u, ['n'] = 0x31u, ['o'] = 0x18u, ['p'] = 0x19u,
        ['q'] = 0x10u, ['r'] = 0x13u, ['s'] = 0x1fu, ['t'] = 0x14u,
        ['u'] = 0x16u, ['v'] = 0x2fu, ['w'] = 0x11u, ['x'] = 0x2du,
        ['y'] = 0x15u, ['z'] = 0x2cu,
        ['-'] = 0x0cu, ['='] = 0x0du, ['['] = 0x1au, [']'] = 0x1bu,
        ['\\'] = 0x2bu, [';'] = 0x27u, ['\''] = 0x28u, ['`'] = 0x29u,
        [','] = 0x33u, ['.'] = 0x34u, ['/'] = 0x35u
    };

    if (value >= sizeof(scan_codes)) return 0u;
    if (value >= 'A' && value <= 'Z') value = value - 'A' + 'a';
    return scan_codes[value];
}

type_status vm_profile_default_keyboard_map_host_key(uint16_t host_scan_code,
    uint16_t host_virtual_key, uint8_t *out_scan_code)
{
    uint8_t scan_code;

    if (out_scan_code == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (host_scan_code > 0u && host_scan_code <= 0x58u) {
        *out_scan_code = (uint8_t)host_scan_code;
        return TYPE_STATUS_OK;
    }
    scan_code = vm_profile_default_keyboard_map_ascii(host_virtual_key);
    if (scan_code == 0u) return TYPE_STATUS_UNSUPPORTED;
    *out_scan_code = scan_code;
    return TYPE_STATUS_OK;
}
