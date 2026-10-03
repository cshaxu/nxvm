#ifndef CORE_MACHINE_DEVICE_SUPPORT_H
#define CORE_MACHINE_DEVICE_SUPPORT_H

#include "lib/types/types_interface.h"

/* Device-local operations retained from the retired root type facade. */
#define CORE_MACHINE_BIT_IS_SET(state, flag) (!!((state) & (flag)))
#define CORE_MACHINE_BIT_SET(destination, source) ((destination) |= (source))
#define CORE_MACHINE_BIT_CLEAR(destination, source) ((destination) &= ~(source))
#define CORE_MACHINE_BIT_MAKE(destination, source, enabled) \
    ((enabled) ? CORE_MACHINE_BIT_SET((destination), (source)) : \
        CORE_MACHINE_BIT_CLEAR((destination), (source)))

#define CORE_MACHINE_MASK_U8(value) ((lib_u8)(value))
#define CORE_MACHINE_MASK_U16(value) ((lib_u16)(value))
#define CORE_MACHINE_MASK_U24(value) ((lib_u32)(value) & 0x00ffffffu)
#define CORE_MACHINE_MASK_U32(value) ((lib_u32)(value))
#define CORE_MACHINE_MASK_U64(value) ((lib_u64)(value))
#define CORE_MACHINE_MASK_U1(value) ((lib_u8)(value) & 1u)

#define CORE_MACHINE_REFERENCE_OF(value) lib_pointer_to_uptr(&(value))
#define CORE_MACHINE_DEREFERENCE_U8(address) (*(lib_u8 *)(lib_uptr)(address))

#define CORE_MACHINE_MSB_7 0x40u
#define CORE_MACHINE_MSB_8 0x80u
#define CORE_MACHINE_MSB_15 0x4000u
#define CORE_MACHINE_MSB_16 0x8000u
#define CORE_MACHINE_MSB_31 0x40000000u
#define CORE_MACHINE_MSB_32 0x80000000u
#define CORE_MACHINE_GET_MSB_7(value) (CORE_MACHINE_MASK_U8(value) & CORE_MACHINE_MSB_7)
#define CORE_MACHINE_GET_MSB_8(value) (CORE_MACHINE_MASK_U8(value) & CORE_MACHINE_MSB_8)
#define CORE_MACHINE_GET_MSB_15(value) (CORE_MACHINE_MASK_U16(value) & CORE_MACHINE_MSB_15)
#define CORE_MACHINE_GET_MSB_16(value) (CORE_MACHINE_MASK_U16(value) & CORE_MACHINE_MSB_16)
#define CORE_MACHINE_GET_MSB_31(value) (CORE_MACHINE_MASK_U32(value) & CORE_MACHINE_MSB_31)
#define CORE_MACHINE_GET_MSB_32(value) (CORE_MACHINE_MASK_U32(value) & CORE_MACHINE_MSB_32)
#define CORE_MACHINE_GET_LSB(value) CORE_MACHINE_BIT_IS_SET((value), 1u)
#define CORE_MACHINE_GET_LSB_U8(value) (CORE_MACHINE_MASK_U8(value) & 1u)
#define CORE_MACHINE_GET_LSB_U16(value) (CORE_MACHINE_MASK_U16(value) & 1u)
#define CORE_MACHINE_GET_LSB_U32(value) (CORE_MACHINE_MASK_U32(value) & 1u)

#endif
