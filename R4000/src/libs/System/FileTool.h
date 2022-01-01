#pragma once

static inline bool FileExists(const char *pPath)
{
  SceIoStat stat;
  int ret=sceIoGetstat(pPath,&stat);
  if(ret<0) return(false);
  return(true);
}

