#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/profile/default_profile/keyboard_mapper.h"

static type_unsigned_8 vm_profile_default_keyboard_map_ascii(type_unsigned_16 value)
{
    static const type_unsigned_8 scan_codes[128] = {
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

static type_unsigned_8 vm_profile_default_keyboard_set1_to_set2(
    type_unsigned_8 set1, type_bool *out_known)
{
    static const type_unsigned_8 map[0x59] = {
        [0x01] = 0x76u, [0x02] = 0x16u, [0x03] = 0x1eu, [0x04] = 0x26u,
        [0x05] = 0x25u, [0x06] = 0x2eu, [0x07] = 0x36u, [0x08] = 0x3du,
        [0x09] = 0x3eu, [0x0a] = 0x46u, [0x0b] = 0x45u, [0x0c] = 0x4eu,
        [0x0d] = 0x55u, [0x0e] = 0x66u, [0x0f] = 0x0du, [0x10] = 0x15u,
        [0x11] = 0x1du, [0x12] = 0x24u, [0x13] = 0x2du, [0x14] = 0x2cu,
        [0x15] = 0x35u, [0x16] = 0x3cu, [0x17] = 0x43u, [0x18] = 0x44u,
        [0x19] = 0x4du, [0x1a] = 0x54u, [0x1b] = 0x5bu, [0x1c] = 0x5au,
        [0x1d] = 0x14u, [0x1e] = 0x1cu, [0x1f] = 0x1bu, [0x20] = 0x23u,
        [0x21] = 0x2bu, [0x22] = 0x34u, [0x23] = 0x33u, [0x24] = 0x3bu,
        [0x25] = 0x42u, [0x26] = 0x4bu, [0x27] = 0x4cu, [0x28] = 0x52u,
        [0x29] = 0x0eu, [0x2a] = 0x12u, [0x2b] = 0x5du, [0x2c] = 0x1au,
        [0x2d] = 0x22u, [0x2e] = 0x21u, [0x2f] = 0x2au, [0x30] = 0x32u,
        [0x31] = 0x31u, [0x32] = 0x3au, [0x33] = 0x41u, [0x34] = 0x49u,
        [0x35] = 0x4au, [0x36] = 0x59u, [0x37] = 0x7cu, [0x38] = 0x11u,
        [0x39] = 0x29u, [0x3a] = 0x58u, [0x3b] = 0x05u, [0x3c] = 0x06u,
        [0x3d] = 0x04u, [0x3e] = 0x0cu, [0x3f] = 0x03u, [0x40] = 0x0bu,
        [0x41] = 0x83u, [0x42] = 0x0au, [0x43] = 0x01u, [0x44] = 0x09u,
        [0x45] = 0x77u, [0x46] = 0x7eu, [0x47] = 0x6cu, [0x48] = 0x75u,
        [0x49] = 0x7du, [0x4a] = 0x7bu, [0x4b] = 0x6bu, [0x4c] = 0x73u,
        [0x4d] = 0x74u, [0x4e] = 0x79u, [0x4f] = 0x69u, [0x50] = 0x72u,
        [0x51] = 0x7au, [0x52] = 0x70u, [0x53] = 0x71u, [0x57] = 0x78u,
        [0x58] = 0x07u
    };

    if (out_known == STD_NULL) return 0u;
    *out_known = set1 < sizeof(map) && map[set1] != 0u;
    return *out_known ? map[set1] : 0u;
}

type_status vm_profile_default_keyboard_map_host_key_for_scan_set(
    type_unsigned_16 host_scan_code, type_unsigned_16 host_virtual_key,
    C_INT pressed, type_unsigned_8 native_scan_set,
    vm_profile_default_keyboard_sequence *out_sequence)
{
    type_bool known;
    type_unsigned_8 scan_code;

    if (out_sequence == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    out_sequence->count = 0u;
    /* Win32 identifies Pause by virtual key. */
    if (host_virtual_key == 0x13u) {
        if (!pressed) return TYPE_STATUS_OK;
        if (native_scan_set == CORE_MACHINE_KEYBOARD_SCAN_SET_1) {
            static const type_unsigned_8 pause_set1[] = {
                0xe1u, 0x1du, 0x45u, 0xe1u, 0x9du, 0xc5u
            };
            STD_MEMCPY(out_sequence->bytes, pause_set1, sizeof(pause_set1));
            out_sequence->count = sizeof(pause_set1);
            return TYPE_STATUS_OK;
        }
        out_sequence->bytes[0] = 0xe1u;
        out_sequence->bytes[1] = 0x14u;
        out_sequence->bytes[2] = 0x77u;
        out_sequence->bytes[3] = 0xe1u;
        out_sequence->bytes[4] = 0xf0u;
        out_sequence->bytes[5] = 0x14u;
        out_sequence->bytes[6] = 0xf0u;
        out_sequence->bytes[7] = 0x77u;
        out_sequence->count = 8u;
        return TYPE_STATUS_OK;
    }
    if ((host_scan_code & 0xffu) > 0u &&
        (host_scan_code & 0xffu) <= 0x58u) {
        scan_code = (type_unsigned_8)(host_scan_code & 0xffu);
    } else {
        scan_code = vm_profile_default_keyboard_map_ascii(host_virtual_key);
        if (scan_code == 0u) return TYPE_STATUS_UNSUPPORTED;
    }
    if (native_scan_set == CORE_MACHINE_KEYBOARD_SCAN_SET_1) {
        if (!pressed) scan_code |= 0x80u;
        if ((host_scan_code & 0x0100u) != 0u) {
            out_sequence->bytes[out_sequence->count++] = 0xe0u;
        }
        out_sequence->bytes[out_sequence->count++] = scan_code;
        return TYPE_STATUS_OK;
    }
    scan_code = vm_profile_default_keyboard_set1_to_set2(scan_code, &known);
    if (!known) return TYPE_STATUS_UNSUPPORTED;
    /* Windows scan-code bit 8 identifies the E0-prefixed key variant. */
    if ((host_scan_code & 0x0100u) != 0u) {
        out_sequence->bytes[out_sequence->count++] = 0xe0u;
    }
    if (!pressed) out_sequence->bytes[out_sequence->count++] = 0xf0u;
    out_sequence->bytes[out_sequence->count++] = scan_code;
    return TYPE_STATUS_OK;
}

type_status vm_profile_default_keyboard_map_host_key(type_unsigned_16 host_scan_code,
    type_unsigned_16 host_virtual_key, C_INT pressed,
    vm_profile_default_keyboard_sequence *out_sequence)
{
    return vm_profile_default_keyboard_map_host_key_for_scan_set(host_scan_code,
        host_virtual_key, pressed, CORE_MACHINE_KEYBOARD_SCAN_SET_2, out_sequence);
}
