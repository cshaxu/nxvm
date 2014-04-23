/* This file is a part of NekoVMac project. */

#ifndef NVM_COPTION_H
#define NVM_COPTION_H

#define NVM_NONE 0

// Length of integer: 16,32,64

// Type of system
#define NVM_SYS_MSDOS 1
#define NVM_SYS_WIN32CON 2
#define NVM_SYS_WIN32APP 3

// Type of c compiler
#define NVM_CC_MSVC 1

// Selector - For User's Information
#define NVM_INTLEN 64
#define NVM_SYSTEM NVM_SYS_WIN32CON
#define NVM_CCOMPILER NVM_CC_MSVC

#endif