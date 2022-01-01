#pragma once

#define FrapNull (-1)

#define fopen err
#define fclose err
#define fseek err
#define fread err
#define fwrite err

static u32 Frap_Offset;

static u32 FrapGetFileSizeFromFilename(const char *pfn)
{
  SceIoStat stat;
  if(sceIoGetstat(pfn,&stat)<0) return(0);
  return(stat.st_size);
}

static SceUID FrapOpenRead(const char *pFilename)
{
  SceUID fp=sceIoOpen(pFilename,PSP_O_RDONLY,0777);
  if(fp<0){
    conout("File open error for read. [%s] ErrorCode=0x%08x.\n",pFilename,fp);
    fp=FrapNull;
  }
  Frap_Offset=0;
  return(fp);
}

static SceUID FrapOpenWrite(const char *pFilename)
{
  SceUID fp=sceIoOpen(pFilename,PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC,0777);
  if(fp<0){
    conout("File open error for write. [%s] ErrorCode=0x%08x.\n",pFilename,fp);
    fp=FrapNull;
  }
  Frap_Offset=0;
  return(fp);
}

static void FrapClose(SceUID fp)
{
  if(0<=fp) sceIoClose(fp);
}

static u32 FrapGetFileSize(SceUID fp)
{
  sceIoLseek32(fp,0,SEEK_SET);
  u32 size=sceIoLseek32(fp,0,SEEK_END);
  sceIoLseek32(fp,Frap_Offset,SEEK_SET);
  return(size);
}

static u32 FrapGetPos(SceUID fp)
{
  return(Frap_Offset);
}

static void FrapSetPos(SceUID fp,u32 pos)
{
  Frap_Offset=pos;
  sceIoLseek32(fp,Frap_Offset,SEEK_SET);
}

static u32 FrapRead(SceUID fp,void *pbuf,u32 size)
{
  u32 readed=sceIoRead(fp,pbuf,size);
  Frap_Offset+=readed;
  return(readed);
}

static void FrapSkip(SceUID fp,u32 skipsize)
{
  if(skipsize==0) return;
  
  FrapSetPos(fp,FrapGetPos(fp)+skipsize);
}

static u32 FrapWrite(SceUID fp,const void *pbuf,u32 size)
{
  u32 writed=sceIoWrite(fp,pbuf,size);
  Frap_Offset+=writed;
  return(writed);
}

