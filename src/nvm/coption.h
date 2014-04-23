/* This file is a part of NekoVMac project. */

#ifndef NVM_COPTION_H
#define NVM_COPTION_H

#define NVM_NONE 0

// Length of integer: 16,32,64

// Type of system
#define NVM_WIN32CON 1
#define NVM_WIN32APP 2

// Type of c compiler
#define NVM_MSVC 1

// Selector - For User's Information
#define NVM_INTLEN 64
#define NVM_SYSTEM NVM_WIN32CON
#define NVM_CCOMPILER NVM_MSVC

#endif