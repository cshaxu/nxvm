/* This file is a part of NXVM project. */

/* C Compiler Options: For User's Information */

#ifndef NXVM_COPTION_H
#define NXVM_COPTION_H

/* Length of integer: 16,32,64 */

/* Type of system */
#define NXVM_NONE 0
#define NXVM_LINUX_TERMINAL 1
#define NXVM_LINUX_APPLICATION 2
#define NXVM_WIN32_CONSOLE 3
#define NXVM_WIN32_APPLICATION 4

/* Selector */
#define NXVM_INTLEN 32
#define NXVM_SYSTEM NXVM_WIN32_CONSOLE

#endif