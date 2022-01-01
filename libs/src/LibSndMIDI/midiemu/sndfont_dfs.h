
// ------------------------------------------------

static FILE *SndFontDFS_fp;

static void SndFontDFS_Init(FILE *fp)
{
  SndFontDFS_fp=fp;
}

static void SndFontDFS_Free(void)
{
  SndFontDFS_fp=NULL;
}

static void SndFontDFS_SetOffset(u32 ofs)
{
  fseek(SndFontDFS_fp,ofs,SEEK_SET);
}

static u32 SndFontDFS_GetOffset(void)
{
  return(ftell(SndFontDFS_fp));
}

static u32 SndFontDFS_Read16bit(void *pbuf,u32 size)
{
  return(fread(pbuf,size,1,SndFontDFS_fp));
}

