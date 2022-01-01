
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "euc2unicode.h"
#include "memtools.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

#include "sceIo_Frap.h"

#include "plug_mp3_id3v2.h"

#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)

u32 ID3v2_GetTagSize(SceUID pf,u32 FileTopOffset)
{
  u32 ID3v2Size=0;
  
  u8 rbuf[10];
  FrapSetPos(pf,FileTopOffset);
  FrapRead(pf,rbuf,10);
  
  u8 ID0=rbuf[0];
  u8 ID1=rbuf[1];
  u8 ID2=rbuf[2];
  u8 Version=rbuf[3];
  u8 Revision=rbuf[4];
  u8 Flag=rbuf[5];
  u32 Size0=rbuf[6+0],Size1=rbuf[6+1],Size2=rbuf[6+2],Size3=rbuf[6+3];
  
  if((ID0==0x49)&&(ID1==0x44)&&(ID2==0x33)&&(Version!=0xff)&&(Revision!=0xff)&&(Size0<0x80)&&(Size1<0x80)&&(Size2<0x80)&&(Size3<0x80)){
    ID3v2Size=(ID3v2Size<<7)|Size0;
    ID3v2Size=(ID3v2Size<<7)|Size1;
    ID3v2Size=(ID3v2Size<<7)|Size2;
    ID3v2Size=(ID3v2Size<<7)|Size3;
    ID3v2Size+=10;
  }
  
  return(ID3v2Size);
}

typedef struct {
  char v22[4],v23[5];
} TFrameID;

#define FrameIDsCount (10)
static const TFrameID FrameIDs[FrameIDsCount]={
{"TT1","TIT1"},
{"TT2","TIT2"},
{"TT3","TIT3"},
{"TP1","TPE1"},
{"TAL","TALB"},
{"TCM","TCOM"},
{"TCR","TCOP"},
{"TOT","TOAL"},
{"TOL","TOLY"},
{"TOA","TOPE"},
};

typedef struct {
  wchar *pText;
} THeaderFrame;

typedef struct {
  u32 Version;
  bool fUnsync,fCompression,fExtHeader,fExperimentalIndicator;
  u32 ExtHeaderSize;
  THeaderFrame Frames[FrameIDsCount];
} THeader;

static bool Loaded=false;
static THeader *pHeader;

bool ID3v2_Loaded(void)
{
  return(Loaded);
}

void ID3v2_Free(void)
{
  if(Loaded==false) return;
  Loaded=false;
  
  if(pHeader!=NULL){
    for(u32 idx=0;idx<FrameIDsCount;idx++){
      if(pHeader->Frames[idx].pText!=NULL){
        safefree(pHeader->Frames[idx].pText); pHeader->Frames[idx].pText=NULL;
      }
    }
    safefree(pHeader); pHeader=NULL;
  }
}

static const u16 cp1252_tbl[256] = {
0xffff,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
0x0008,0x0009,0x000a,0x000b,0x000c,0x000d,0x000e,0x000f,
0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
0x0018,0x0019,0x001a,0x001b,0x001c,0x001d,0x001e,0x001f,
0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
0x0028,0x0029,0x002a,0x002b,0x002c,0x002d,0x002e,0x002f,
0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
0x0038,0x0039,0x003a,0x003b,0x003c,0x003d,0x003e,0x003f,
0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
0x0048,0x0049,0x004a,0x004b,0x004c,0x004d,0x004e,0x004f,
0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
0x0058,0x0059,0x005a,0x005b,0x005c,0x005d,0x005e,0x005f,
0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
0x0068,0x0069,0x006a,0x006b,0x006c,0x006d,0x006e,0x006f,
0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
0x0078,0x0079,0x007a,0x007b,0x007c,0x007d,0x007e,0x007f,
0x20ac,0xffff,0x201a,0x0192,0x201e,0x2026,0x2020,0x2021,
0x02c6,0x2030,0x0160,0x2039,0x0152,0xffff,0x017d,0xffff,
0xffff,0x2018,0x2019,0x201c,0x201d,0x2022,0x2013,0x2014,
0x02dc,0x2122,0x0161,0x203a,0x0153,0xffff,0x017e,0x0178,
0x00a0,0x00a1,0x00a2,0x00a3,0x00a4,0x00a5,0x00a6,0x00a7,
0x00a8,0x00a9,0x00aa,0x00ab,0x00ac,0x00ad,0x00ae,0x00af,
0x00b0,0x00b1,0x00b2,0x00b3,0x00b4,0x00b5,0x00b6,0x00b7,
0x00b8,0x00b9,0x00ba,0x00bb,0x00bc,0x00bd,0x00be,0x00bf,
0x00c0,0x00c1,0x00c2,0x00c3,0x00c4,0x00c5,0x00c6,0x00c7,
0x00c8,0x00c9,0x00ca,0x00cb,0x00cc,0x00cd,0x00ce,0x00cf,
0x00d0,0x00d1,0x00d2,0x00d3,0x00d4,0x00d5,0x00d6,0x00d7,
0x00d8,0x00d9,0x00da,0x00db,0x00dc,0x00dd,0x00de,0x00df,
0x00e0,0x00e1,0x00e2,0x00e3,0x00e4,0x00e5,0x00e6,0x00e7,
0x00e8,0x00e9,0x00ea,0x00eb,0x00ec,0x00ed,0x00ee,0x00ef,
0x00f0,0x00f1,0x00f2,0x00f3,0x00f4,0x00f5,0x00f6,0x00f7,
0x00f8,0x00f9,0x00fa,0x00fb,0x00fc,0x00fd,0x00fe,0x00ff
};

typedef struct {
  u32 ID;
  u32 Size;
  u8 StatusFlag,FormatFlag;
} TFrame;

static wchar* LoadFrame_ins_GetText(u32 FrameIDIndex,u8 *pbuf,u32 bufsize)
{
  u32 bufofs=0;
  
#define R8(x) { (x)=pbuf[bufofs+0]; bufofs+=1; }
#define R16(x) { (x)=(pbuf[bufofs+0]<<8)|(pbuf[bufofs+1]<<0); bufofs+=2; }
#define R24(x) { (x)=(pbuf[bufofs+0]<<16)|(pbuf[bufofs+1]<<8)|(pbuf[bufofs+2]<<0); bufofs+=3; }
#define R32(x) { (x)=(pbuf[bufofs+0]<<24)|(pbuf[bufofs+1]<<16)|(pbuf[bufofs+2]<<8)|(pbuf[bufofs+3]<<0); bufofs+=4; }
#define R16BE(x) { (x)=(pbuf[bufofs+0]<<0)|(pbuf[bufofs+1]<<8); bufofs+=2; }
#define R24BE(x) { (x)=(pbuf[bufofs+0]<<0)|(pbuf[bufofs+1]<<8)|(pbuf[bufofs+2]<<16); bufofs+=3; }
#define R32BE(x) { (x)=(pbuf[bufofs+0]<<0)|(pbuf[bufofs+1]<<8)|(pbuf[bufofs+2]<<16)|(pbuf[bufofs+3]<<24); bufofs+=4; }

  u8 Encode;
  R8(Encode);
  
  wchar *pdstw=NULL;
  
  if(Encode==0x00){ // Latin-1 CP1252
    if(EUC2Unicode_isLoaded()==false){
      conout("ID3TagV2: is Latin-1 CP1252.\n");
      s32 length=bufsize-1;
      if(0<length){
        pdstw=(wchar*)safemalloc_chkmem((length+1)*2);
        for(u32 idx=0;idx<length;idx++){
          u8 lc;
          R8(lc);
          wchar wc=cp1252_tbl[lc];
          if(wc==0xffff) wc=0;
          pdstw[idx]=wc;
        }
        pdstw[length]=0;
      }
      }else{
      conout("ID3TagV2: is EUC/S-JIS.\n");
      s32 length=bufsize-1;
      if(0<length){
        char *psrc=(char*)safemalloc_chkmem(length+1);
        for(u32 idx=0;idx<length;idx++){
          R8(psrc[idx]);
        }
        psrc[length]=0;
        pdstw=EUC2Unicode_Convert(psrc);
        safefree(psrc); psrc=NULL;
      }
    }
  }
  
  if(Encode==0x01){ // Unicode16 with BOM
    u16 BOM;
    R16(BOM);
    s32 bomsize=2;
    bool isBE;
    if(BOM==0xfeff){
      conout("ID3TagV2: is UTF-16LE.\n");
      isBE=false;
      }else{
      if(BOM==0xfffe){
        conout("ID3TagV2: is UTF-16BE.\n");
        isBE=true;
        }else{
        conout("ID3TagV2: Can not found BOM. Set to UTF-16LE\n");
        isBE=false;
        bufofs-=2;
        bomsize=0;
      }
    }
    s32 length=(bufsize-bomsize-1)/2;
    if(0<length){
      pdstw=(wchar*)safemalloc_chkmem((length+1)*2);
      for(u32 idx=0;idx<length;idx++){
        wchar wc;
        if(isBE==false){
          R16(wc);
          }else{
          R16BE(wc);
        }
        pdstw[idx]=wc;
      }
      pdstw[length]=0;
    }
  }
  
#undef R8
#undef R16
#undef R24
#undef R32
#undef R16BE
#undef R24BE
#undef R32BE
  
  if((pdstw!=NULL)&&(pdstw[0]!=0)) return(pdstw);
  
  if(pdstw!=NULL){
    safefree(pdstw); pdstw=NULL;
  }
  
  return(NULL);
}

static void LoadFrame(u32 ID,u32 filepos,u8 *pbuf,u32 bufsize)
{
  char *pIDstr=(char*)&ID;
  
  if(((pIDstr[0]=='P')&&(pIDstr[1]=='I')&&(pIDstr[2]=='C'))||((pIDstr[0]=='A')&&(pIDstr[1]=='P')&&(pIDstr[2]=='I')&&(pIDstr[3]=='C'))){
    for(u32 idx=0;idx<128;idx++){
      bool found=false;
      if((pbuf[idx+0]==0x89)&&(pbuf[idx+1]==0x50)&&(pbuf[idx+2]==0x4E)&&(pbuf[idx+3]==0x47)) found=true; // png
      if((pbuf[idx+0]==0xFF)&&(pbuf[idx+1]==0xD8)&&(pbuf[idx+2]==0xFF)) found=true; // jpeg
      if(found==true){
        ArtWork_Offset=filepos+idx;
        ArtWork_Size=bufsize-idx;
        conout("ID3TagV2: Found art-work image. (%d:%d:0x%x)\n",ArtWork_Offset,ArtWork_Size,ArtWork_Offset+ArtWork_Size);
        return;
      }
    }
    conout("ID3TagV2: Unknown art-work image format.\n");
    return;
  }
  
  u32 FrameIDIndex=(u32)-1;
  for(u32 idx=0;idx<FrameIDsCount;idx++){
    bool v22found=true;
    if(pIDstr[0]!=FrameIDs[idx].v22[0]) v22found=false;
    if(pIDstr[1]!=FrameIDs[idx].v22[1]) v22found=false;
    if(pIDstr[2]!=FrameIDs[idx].v22[2]) v22found=false;
    bool v23found=true;
    if(pIDstr[0]!=FrameIDs[idx].v23[0]) v23found=false;
    if(pIDstr[1]!=FrameIDs[idx].v23[1]) v23found=false;
    if(pIDstr[2]!=FrameIDs[idx].v23[2]) v23found=false;
    if(pIDstr[3]!=FrameIDs[idx].v23[3]) v23found=false;
    if((v22found==true)||(v23found==true)){
      FrameIDIndex=idx;
      break;
    }
  }
  
  if(FrameIDIndex==(u32)-1){
    conout("ID3TagV2: Skip this frame.\n");
    return;
  }
  
  if(pHeader->Frames[FrameIDIndex].pText!=NULL){
    conout("ID3TagV2: Skip duplicate frame.\n");
    return;
  }
  
  pHeader->Frames[FrameIDIndex].pText=LoadFrame_ins_GetText(FrameIDIndex,pbuf,bufsize);
}

void ID3v2_Load(SceUID pf,u32 FileTopOffset)
{
  ID3v2_Free();
  
  u32 ID3v2Size=0;
  
  u8 rbuf[10];
  FrapSetPos(pf,FileTopOffset);
  FrapRead(pf,rbuf,10);
  
  u8 ID0=rbuf[0];
  u8 ID1=rbuf[1];
  u8 ID2=rbuf[2];
  u8 Version=rbuf[3];
  u8 Revision=rbuf[4];
  u8 Flag=rbuf[5];
  u32 Size0=rbuf[6+0],Size1=rbuf[6+1],Size2=rbuf[6+2],Size3=rbuf[6+3];
  
  if((ID0==0x49)&&(ID1==0x44)&&(ID2==0x33)&&(Version!=0xff)&&(Revision!=0xff)&&(Size0<0x80)&&(Size1<0x80)&&(Size2<0x80)&&(Size3<0x80)){
    ID3v2Size=(ID3v2Size<<7)|Size0;
    ID3v2Size=(ID3v2Size<<7)|Size1;
    ID3v2Size=(ID3v2Size<<7)|Size2;
    ID3v2Size=(ID3v2Size<<7)|Size3;
  }
  
  if(ID3v2Size==0) return;
  
  if((Version!=0x02)&&(Version!=0x03)){ // ID3TagV2.2 and ID3TagV2.3 only.
    conout("ID3TagV2: Not support Version%d.\n",Version);
    return;
  }
  
  conout("ID3TagV2: Load version%d.\n",Version);
  
  Loaded=true;
  
  pHeader=(THeader*)safemalloc_chkmem(sizeof(THeader));
  pHeader->Version=Version;
  
  for(u32 idx=0;idx<FrameIDsCount;idx++){
    pHeader->Frames[idx].pText=NULL;
  }
  
  pHeader->fUnsync=false;
  if((Flag&BIT7)!=0){
    pHeader->fUnsync=true;
    conout("ID3TagV2: is Unsynchronisation.\n");
  }
  pHeader->fCompression=false;
  if(pHeader->Version==0x02){
    if((Flag&BIT6)!=0){
      pHeader->fCompression=true;
      conout("ID3TagV2: is Compression.\n");
    }
  }
  pHeader->fExtHeader=false;
  if(pHeader->Version==0x03){
    if((Flag&BIT6)!=0){
      pHeader->fExtHeader=true;
      conout("ID3TagV2: is Extended header.\n");
    }
  }
  pHeader->fExperimentalIndicator=false;
  if((Flag&BIT5)!=0){
    pHeader->fExperimentalIndicator=true;
    conout("ID3TagV2: is Experimental indicator.\n");
  }
  
  if(pHeader->fCompression==true){ // not support compressed tag.
    conout("ID3TagV2: Not support compressed tag.\n");
    ID3v2_Free();
    return;
  }
  
  u8 *pbuf=(u8*)safemalloc_chkmem(ID3v2Size);
  u32 bufofs=0,bufsize=ID3v2Size;
  if(FrapRead(pf,pbuf,bufsize)!=ID3v2Size){
    conout("ID3v2Size = %dbyte. file size error.\n",ID3v2Size);
    if(pbuf!=NULL){
      safefree(pbuf); pbuf=NULL;
    }
    ID3v2_Free();
    return;
  }
  
  if(pHeader->fUnsync==true){
    conout("ID3TagV2: Unsynchronisation processing.\n");
    u32 srcpos=0,dstpos=0;
    while(1){
      if(bufsize<=(srcpos+2)) break;
      if((pbuf[srcpos+0]==0xff)&&(pbuf[srcpos+1]==0x00)){
        pbuf[dstpos+0]=pbuf[srcpos+0];
        pbuf[dstpos+1]=pbuf[srcpos+2];
        srcpos+=3;
        dstpos+=2;
        }else{
        pbuf[dstpos++]=pbuf[srcpos++];
      }
    }
    if(srcpos<bufsize) pbuf[dstpos++]=pbuf[srcpos++];
    if(srcpos<bufsize) pbuf[dstpos++]=pbuf[srcpos++];
    bufsize=dstpos;
  }
  
#define R8(x) { (x)=pbuf[bufofs+0]; bufofs+=1; }
#define R16(x) { (x)=(pbuf[bufofs+0]<<8)|(pbuf[bufofs+1]<<0); bufofs+=2; }
#define R24(x) { (x)=(pbuf[bufofs+0]<<16)|(pbuf[bufofs+1]<<8)|(pbuf[bufofs+2]<<0); bufofs+=3; }
#define R32(x) { (x)=(pbuf[bufofs+0]<<24)|(pbuf[bufofs+1]<<16)|(pbuf[bufofs+2]<<8)|(pbuf[bufofs+3]<<0); bufofs+=4; }
#define R16BE(x) { (x)=(pbuf[bufofs+0]<<0)|(pbuf[bufofs+1]<<8); bufofs+=2; }
#define R24BE(x) { (x)=(pbuf[bufofs+0]<<0)|(pbuf[bufofs+1]<<8)|(pbuf[bufofs+2]<<16); bufofs+=3; }
#define R32BE(x) { (x)=(pbuf[bufofs+0]<<0)|(pbuf[bufofs+1]<<8)|(pbuf[bufofs+2]<<16)|(pbuf[bufofs+3]<<24); bufofs+=4; }

  if(pHeader->fExtHeader==true){
    u32 ExtHeaderSize;
    R32(ExtHeaderSize);
    conout("ID3TagV2: Skip Extended header. %dbyte.\n",ExtHeaderSize);
    bufofs+=ExtHeaderSize;
  }
  
  while(bufofs<bufsize){
    TFrame Frame;
    if(pHeader->Version==0x02){
      R24BE(Frame.ID);
      R24(Frame.Size);
      Frame.StatusFlag=0;
      Frame.FormatFlag=0;
    }
    if(pHeader->Version==0x03){
      R32BE(Frame.ID);
      R32(Frame.Size);
      R8(Frame.StatusFlag);
      R8(Frame.FormatFlag);
    }
    bool skip=false;
    if((Frame.FormatFlag&BIT7)!=0){ // compression
      skip=true;
      u32 dummy;
      R32(dummy);
    }
    if((Frame.FormatFlag&BIT6)!=0){ // encrypted
      skip=true;
      u8 dummy;
      R8(dummy);
    }
    u32 NextFramePos=bufofs+Frame.Size;
    if((skip==false)&&(Frame.Size!=0)){
      {
        u32 tmp[2];
        tmp[0]=Frame.ID;
        tmp[1]=0;
        conout("ID3TagV2: Found [%s] frame. ofs=%dbyte. size=%dbyte.\n",tmp,bufofs,Frame.Size);
      }
      LoadFrame(Frame.ID,10+bufofs,&pbuf[bufofs],Frame.Size);
    }
    bufofs=NextFramePos;
  }
  
#undef R8
#undef R16
#undef R24
#undef R32
#undef R16BE
#undef R24BE
#undef R32BE

  if(pbuf!=NULL){
    safefree(pbuf); pbuf=NULL;
  }
}

int ID3v2_GetInfoIndexCount(void)
{
  if(ID3v2_Loaded()==false) return(0);
  
  return(FrameIDsCount);
}

bool ID3v2_GetInfoStrW(int idx,wchar *str,int len)
{
  if(str==NULL) return(false);
  str[0]=0;
  
  if(ID3v2_Loaded()==false) return(false);
  
  const wchar *pw=pHeader->Frames[idx].pText;
  if(pw==NULL) return(false);
  
  for(u32 idx=0;idx<len;idx++){
    str[idx]=pw[idx];
  }
  str[len]=0;
  return(true);
}

u32 ID3v2_GetInfoStrLen(int idx)
{
  if(ID3v2_Loaded()==false) return(0);
  
  const wchar *pw=pHeader->Frames[idx].pText;
  if(pw==NULL) return(0);
  
  u32 len=0;
  while(1){
    wchar w=*pw++;
    if(w==0) break;
    len++;
  }
  
  return(len);
}

