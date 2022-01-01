#pragma once

#include "Ap4Tag.h"

extern bool AP4_Packet_Init(FILE *pf);
extern void AP4_Packet_Free(void);

extern u32 AP4_Packet_GetSampleRate(void);
extern u32 AP4_Packet_GetChannels(void);
extern u32 AP4_Packet_GetBitsPerSample(void);
extern u32 AP4_Packet_GetAvgBitrate(void);
extern u32 AP4_Packet_GetMaxBitrate(void);

extern bool AP4_Packet_GetData(u8 **ppbuf,u32 *pbufsize);

extern u32 AP4_Packet_GetTotalIndex(void);
extern void AP4_Packet_SetIndex(u32 index);
extern u32 AP4_Packet_GetIndex(void);

