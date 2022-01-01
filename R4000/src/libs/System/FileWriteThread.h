#pragma once

extern void FileWriteThread_Init(void);
extern void FileWriteThread_Free(void);
extern bool FileWriteThread_isExecute(void);
extern void FileWriteThread_Stack(const char *pfn,const u32 size,const void *pbuf);

