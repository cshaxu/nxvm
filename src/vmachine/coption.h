/* This file is a part of NXVM project. */

// C Compiler Options: For User's Information

#ifndef NXVM_COPTION_H
#define NXVM_COPTION_H

// Length of integer: 16,32,64

// Type of system: NONE,LINUX_TERMINAL,LINUX_APPLICATION,WIN32_CONSOLE,WIN32_APPLICATION
#define NXVM_NONE 0
#define NXVM_LINUX_TERMINAL 1
#define NXVM_LINUX_APPLICATION_QT 2
#define NXVM_WIN32_CONSOLE 3
#define NXVM_WIN32_APPLICATION 4

// Selector
#define NXVM_INTLEN 64
#define NXVM_SYSTEM NXVM_NONE

#endif