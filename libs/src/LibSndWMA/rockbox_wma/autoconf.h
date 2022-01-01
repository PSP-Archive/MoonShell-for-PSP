/* fake autoconf for fat testing */

#ifndef __BUILD_AUTOCONF_H
#define __BUILD_AUTOCONF_H

/* assume little endian for now */
#define ROCKBOX_LITTLE_ENDIAN 1

#define regs 

#define __PCTOOL__ 1

#define ICONST_ATTR 

#define intptr_t    long
#define uintptr_t   unsigned long

#define MEM_ALIGN_ATTR __attribute__((aligned(16)))

#include <pspuser.h>
#include "conout.h"

#endif
