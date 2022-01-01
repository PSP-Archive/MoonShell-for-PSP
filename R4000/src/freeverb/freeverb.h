#pragma once

#include <pspuser.h>

extern void* create_freeverb(void);
extern void delete_freeverb(void* self);
extern void setlevel_freeverb(void* self, float level);
extern void process_freeverb_s16(void* self, s16 *in_data, int samplescount);
extern void process_freeverb_s32(void* self, s32 *in_data, int samplescount);

