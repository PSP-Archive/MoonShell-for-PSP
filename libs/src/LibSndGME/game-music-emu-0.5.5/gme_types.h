#ifndef GME_TYPES_H
#define GME_TYPES_H

/* CMake will either define the following to 1, or #undef it,
 * depending on the options passed to CMake.  This is used to
 * conditionally compile in the various emulator types.
 *
 * See gme_type_list() in gme.cpp
 */

#define USE_GME_AY
#define USE_GME_GBS
#define USE_GME_GYM
#define USE_GME_HES
#define USE_GME_KSS
#define USE_GME_NSF
#define USE_GME_NSFE
#define USE_GME_SAP
#define USE_GME_SPC
/* VGM and VGZ are a package deal */
#define USE_GME_VGM

#if 0
#undef USE_GME_AY
#undef USE_GME_GBS
#undef USE_GME_GYM
#undef USE_GME_HES
#undef USE_GME_KSS
#undef USE_GME_NSFE
#undef USE_GME_SAP
#undef USE_GME_SPC
#undef USE_GME_VGM
#endif

#endif /* GME_TYPES_H */
