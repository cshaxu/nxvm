/* Copyright 2012-2014 Neko. */

/* QDKEYB implements quick and dirty keyboard control routines. */

#include "core/product/utils.h"
#include "core/machine/port.h"
#include "core/machine/cpu.h"
#include "core/machine/vpic.h"

#include "vm/profile/default_profile/firmware/qdx.h"
#include "qdkeyb.h"

#define bufptrHead (vramRealWord(Zero16, QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD))
#define bufptrTail (vramRealWord(Zero16, QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL))
#define bufGetSize (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END - \
                    QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1)
#define bufIsEmpty (bufptrHead == bufptrTail)
#define bufIsFull  ((bufptrHead - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START) == \
    (bufptrTail - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 2) % bufGetSize)
#define bufptrAdvance(ptr) ((ptr) = (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + \
        ((ptr) - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 2) % bufGetSize))

static t_bool bufPush(t_nubit16 code) {
    if (bufIsFull) {
        return True;
    }
    vramRealWord(Zero16, bufptrTail) = code;
    bufptrAdvance(bufptrTail);
    return False;
}
static t_nubit16 bufPop() {
    t_nubit16 res = 0;
    if (bufIsEmpty) {
        return res;
    }
    res = vramRealWord(Zero16, bufptrHead);
    bufptrAdvance(bufptrHead);
    return res;
}
static t_nubit16 bufPeek() {
    return vramRealWord(Zero16, bufptrHead);
}

static int qdkeybGetModifier(void *context,
    core_machine_keyboard_modifier modifier);
static void qdkeybApplyHostState(void *context, uint32_t asynchronous_keys,
    uint32_t toggle_keys);
static void qdkeybReceiveKeyPress(void *context, uint16_t code);

static const core_machine_keyboard_provider qdkeybProvider = {
    qdkeybGetModifier,
    qdkeybApplyHostState,
    qdkeybReceiveKeyPress
};

static void qdkeybReadInput() {
    /* TODO: this should have been working with INT 15 */
    while (bufIsEmpty) {
        utilsSleep(10);
    }
    vcpu.data.ax = bufPop();
    vpicSetIRQ(0x01);
}
static void qdkeybGetStatus() {
    t_nubit16 x = bufPeek();
    if (bufIsEmpty) {
        _SetEFLAGS_ZF;
    } else {
        switch (x) {
        case 0x1d00:
        case 0x2a00:
        case 0x3800:
            vcpu.data.ax = Zero16;
            break;
        default:
            vcpu.data.ax = x;
            break;
        }
        _ClrEFLAGS_ZF;
    }
}
static void qdkeybGetShiftStatus() {
    vcpu.data.al = qdkeybVarFlag0;
}
static void qdkeybBufferKey() {
    vcpu.data.al = bufPush((vcpu.data.ch << 8) | vcpu.data.cl);
}

static void INT_09() {
    vport.data.ioByte = 0x20;
    vportExecWrite(0x20);
}
static void INT_16() {
    switch (vcpu.data.ah) {
    case 0x00:
    case 0x10:
        qdkeybReadInput();
        break;
    case 0x01:
    case 0x11:
        qdkeybGetStatus();
        break;
    case 0x02:
        qdkeybGetShiftStatus();
        break;
    case 0x05:
        qdkeybBufferKey();
        break;
    default:
        break;
    }
}

void qdkeybInit() {
    qdxTable[0x09] = (t_faddrcc) INT_09; /* hard keyb */
    qdxTable[0x16] = (t_faddrcc) INT_16; /* soft keyb */
}

int qdkeybGetFlag0CapsLock() {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);
}
int qdkeybGetFlag0NumLock()  {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);
}
int qdkeybGetFlag0Shift() {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT) || GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);
}
int qdkeybGetFlag0Alt()  {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);
}
int qdkeybGetFlag0Ctrl() {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);
}
void qdkeybClrFlag0() {
    qdkeybVarFlag0 = Zero8;
}
void qdkeybClrFlag1() {
    qdkeybVarFlag1 = Zero8;
}

void qdkeybSetFlag0Insert()     {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_INSERT);
}
void qdkeybSetFlag0CapLck()     {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);
}
void qdkeybSetFlag0NumLck()     {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);
}
void qdkeybSetFlag0ScrLck()     {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_SCRLCK);
}
void qdkeybSetFlag0Alt()        {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);
}
void qdkeybSetFlag0Ctrl()       {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);
}
void qdkeybSetFlag0LeftShift()  {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT);
}
void qdkeybSetFlag0RightShift() {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);
}

void qdkeybClrFlag0Insert()     {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_INSERT);
}
void qdkeybClrFlag0CapLck()     {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);
}
void qdkeybClrFlag0NumLck()     {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);
}
void qdkeybClrFlag0ScrLck()     {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_SCRLCK);
}
void qdkeybClrFlag0Alt()        {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);
}
void qdkeybClrFlag0Ctrl()       {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);
}
void qdkeybClrFlag0LeftShift()  {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT);
}
void qdkeybClrFlag0RightShift() {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);
}

void qdkeybSetFlag1Insert()   {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_INSERT);
}
void qdkeybSetFlag1CapLck()   {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_CAPLCK);
}
void qdkeybSetFlag1NumLck()   {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_NUMLCK);
}
void qdkeybSetFlag1ScrLck()   {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SCRLCK);
}
void qdkeybSetFlag1Pause()    {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_PAUSE);
}
void qdkeybSetFlag1SysRq()    {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SYSRQ);
}
void qdkeybSetFlag1LeftAlt()  {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LALT);
}
void qdkeybSetFlag1LeftCtrl() {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LCTRL);
}

void qdkeybClrFlag1Insert()   {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_INSERT);
}
void qdkeybClrFlag1CapLck()   {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_CAPLCK);
}
void qdkeybClrFlag1NumLck()   {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_NUMLCK);
}
void qdkeybClrFlag1ScrLck()   {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SCRLCK);
}
void qdkeybClrFlag1Pause()    {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_PAUSE);
}
void qdkeybClrFlag1SysRq()    {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SYSRQ);
}
void qdkeybClrFlag1LeftAlt()  {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LALT);
}
void qdkeybClrFlag1LeftCtrl() {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LCTRL);
}

void qdkeybRecvKeyPress(uint16_t code) {
    /* while(bufPush(code)) {
        utilsSleep(1);
    } */
    bufPush(code);
    vpicSetIRQ(0x01);
}

static int qdkeybGetModifier(void *context,
    core_machine_keyboard_modifier modifier)
{
    (void)context;
    switch (modifier) {
    case CORE_MACHINE_KEYBOARD_MODIFIER_ALT:
        return qdkeybGetFlag0Alt();
    case CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL:
        return qdkeybGetFlag0Ctrl();
    case CORE_MACHINE_KEYBOARD_MODIFIER_SHIFT:
        return qdkeybGetFlag0Shift();
    case CORE_MACHINE_KEYBOARD_MODIFIER_CAPS_LOCK:
        return qdkeybGetFlag0CapsLock();
    case CORE_MACHINE_KEYBOARD_MODIFIER_NUM_LOCK:
        return qdkeybGetFlag0NumLock();
    }
    return False;
}

static void qdkeybApplyHostState(void *context, uint32_t asynchronous_keys,
    uint32_t toggle_keys)
{
#define QDKEYB_SET_HOST_FLAG(mask, set_call, clear_call) \
    do { if ((mask) != 0u) set_call(); else clear_call(); } while (0)
    (void)context;
    QDKEYB_SET_HOST_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_RIGHT_SHIFT, qdkeybSetFlag0RightShift, qdkeybClrFlag0RightShift);
    QDKEYB_SET_HOST_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_LEFT_SHIFT, qdkeybSetFlag0LeftShift, qdkeybClrFlag0LeftShift);
    QDKEYB_SET_HOST_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_CONTROL, qdkeybSetFlag0Ctrl, qdkeybClrFlag0Ctrl);
    QDKEYB_SET_HOST_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_ALT, qdkeybSetFlag0Alt, qdkeybClrFlag0Alt);
    QDKEYB_SET_HOST_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_SCROLL_LOCK, qdkeybSetFlag1ScrLck, qdkeybClrFlag1ScrLck);
    QDKEYB_SET_HOST_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_NUM_LOCK, qdkeybSetFlag1NumLck, qdkeybClrFlag1NumLck);
    QDKEYB_SET_HOST_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_CAPS_LOCK, qdkeybSetFlag1CapLck, qdkeybClrFlag1CapLck);
    QDKEYB_SET_HOST_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_INSERT, qdkeybSetFlag1Insert, qdkeybClrFlag1Insert);
    QDKEYB_SET_HOST_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_SCROLL_LOCK, qdkeybSetFlag0ScrLck, qdkeybClrFlag0ScrLck);
    QDKEYB_SET_HOST_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_NUM_LOCK, qdkeybSetFlag0NumLck, qdkeybClrFlag0NumLck);
    QDKEYB_SET_HOST_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_CAPS_LOCK, qdkeybSetFlag0CapLck, qdkeybClrFlag0CapLck);
    QDKEYB_SET_HOST_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_INSERT, qdkeybSetFlag0Insert, qdkeybClrFlag0Insert);
    QDKEYB_SET_HOST_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_PAUSE, qdkeybSetFlag1Pause, qdkeybClrFlag1Pause);
#undef QDKEYB_SET_HOST_FLAG
}

static void qdkeybReceiveKeyPress(void *context, uint16_t code)
{
    (void)context;
    qdkeybRecvKeyPress(code);
}

const core_machine_keyboard_provider *vm_profile_default_keyboard_provider(void)
{
    return &qdkeybProvider;
}
