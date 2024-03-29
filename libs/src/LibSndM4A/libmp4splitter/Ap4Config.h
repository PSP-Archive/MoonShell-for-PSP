#pragma once


/*****************************************************************
|
|    AP4 - Target Platform and Compiler Configuration
|
|    Copyright 2002 Gilles Boccon-Gibod
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#include <pspuser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define THROW(x) { conout("Stop exception!! (%d)\n",x); abort(); }

#define AP4_DEBUG

#include <math.h>

#include "AP4_String.h"

#include "common.h"
#include "memtools.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

/*----------------------------------------------------------------------
|       defaults
+---------------------------------------------------------------------*/
#define AP4_CONFIG_HAVE_CPP_STRING_H
//#define AP4_CONFIG_HAVE_STDIO_H
#define AP4_CONFIG_HAVE_ASSERT_H

#define AP4_CONFIG_HAVE_SNPRINTF

/*----------------------------------------------------------------------
|       byte order
+---------------------------------------------------------------------*/
// define AP4_PLATFORM_BYTE_ORDER to one of these two choices
#define AP4_PLATFORM_BYTE_ORDER_BIG_ENDIAN    0
#define AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN 1

#ifdef __ppc__
#define AP4_PLATFORM_BYTE_ORDER AP4_PLATFORM_BYTE_ORDER_BIG_ENDIAN
#endif

#define AP4_PLATFORM_BYTE_ORDER AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN

/*----------------------------------------------------------------------
|       Win32 specifics
+---------------------------------------------------------------------*/
#ifdef WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#if defined(_DEBUG)
#define AP4_DEBUG
#endif
#endif

