#pragma once


/* jconfig.cfg --- source file edited by configure script */
/* see jconfig.doc for explanations */

#undef HAVE_PROTOTYPES
#undef HAVE_UNSIGNED_CHAR
#undef HAVE_UNSIGNED_SHORT
#undef void
#undef const
#undef CHAR_IS_UNSIGNED
#undef HAVE_STDDEF_H
#undef HAVE_STDLIB_H
#undef NEED_BSD_STRINGS
#undef NEED_SYS_TYPES_H
#undef NEED_FAR_POINTERS
#undef NEED_SHORT_EXTERNAL_NAMES
/* Define this if you get warnings about undefined structures. */
#undef INCOMPLETE_TYPES_BROKEN

#ifdef JPEG_INTERNALS

#undef RIGHT_SHIFT_IS_UNSIGNED
#undef INLINE
/* These are for configuring the JPEG memory manager. */
#undef DEFAULT_MAX_MEM
#undef NO_MKTEMP

#endif /* JPEG_INTERNALS */

#define HAVE_STDLIB_H
#define NO_GETENV
#define HAVE_PROTOTYPES

#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
//#define CHAR_IS_UNSIGNED

#define void void
#define const const
#define INLINE inline

#define NO_MKTEMP
#define DEFAULT_MAX_MEM (4*1024*1024)

#define JDCT_DEFAULT  JDCT_IFAST
#define JDCT_FASTEST  JDCT_IFAST

