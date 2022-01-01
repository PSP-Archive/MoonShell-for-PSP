
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>
//#include <pspdisplay.h>

#include "common.h"
#include "euc2unicode.h"
#include "strtool.h"
#include "memtools.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

// ------------------------------------------------------------------------------------

#include "mdxplay-20070206/mdx.h"
#include "mdxplay-20070206/version.h"

static int self_construct(void);
static void self_destroy(void);

static PDX_DATA* _get_pdx(MDX_DATA* mdx, const u8 *ppdxbuf,const u32 pdxbufsize)
{
  mdx->pdx_enable = FLAG_FALSE;
  if ( mdx->haspdx == FLAG_FALSE ){
    mdx->haspdx = FLAG_FALSE;
    mdx->tracks = 9;
    mdx->pdx_enable = FLAG_TRUE;
    return NULL;
  }
  
  PDX_DATA* pdx=mdx_open_pdx(ppdxbuf,pdxbufsize);
  if ( pdx == NULL ){
    LibSnd_DebugOut("PDX file load failed.\n");
    SystemHalt();
  }
  
  LibSnd_DebugOut("PDX file loaded.\n");
  LibSnd_DebugOut("\n");
  mdx->pdx_enable = FLAG_TRUE;
  
  return(pdx);
}

/* class initializations */
extern void* _mdxmml_ym2151_initialize(void);
extern void* _mdx2151_initialize(void);
extern void* _pcm8_initialize(void);
extern void* _ym2151_c_initialize(void);
extern void  _mdxmml_ym2151_finalize(void* self);
extern void  _mdx2151_finalize(void* in_self);
extern void  _pcm8_finalize(void* in_self);
extern void  _ym2151_c_finalize(void* in_self);

static void* self_mdx2151 = NULL;
static void* self_mdxmml_ym2151 = NULL;
static void* self_pcm8 = NULL;
static void* self_ym2151_c = NULL;

void*
_get_mdx2151(void)
{
  return self_mdx2151;
}

void*
_get_mdxmml_ym2151(void)
{
  return self_mdxmml_ym2151;
}

void*
_get_pcm8(void)
{
  return self_pcm8;
}

void*
_get_ym2151_c(void)
{
  return self_ym2151_c;
}

static int
self_construct(void)
{
  self_mdx2151 = _mdx2151_initialize();
  if (!self_mdx2151) {
    goto error_end;
  }
  self_mdxmml_ym2151 = _mdxmml_ym2151_initialize();
  if (!self_mdxmml_ym2151) {
    goto error_end;
  }
  self_pcm8 = _pcm8_initialize();
  if (!self_pcm8) {
    goto error_end;
  }
#if 0
  self_ym2151_c = _ym2151_c_initialize();
  if (!self_ym2151_c) {
    goto error_end;
  }
#endif

  return FLAG_TRUE;

error_end:
#if 0
  if (self_ym2151_c) {
    _ym2151_c_finalize(self_ym2151_c);
    self_ym2151_c = NULL;
  }
#endif
  if (self_pcm8) {
    _pcm8_finalize(self_pcm8);
    self_pcm8 = NULL;
  }
  if (self_mdxmml_ym2151) {
    _mdxmml_ym2151_finalize(self_mdxmml_ym2151);
    self_mdxmml_ym2151 = NULL;
  }
  if (self_mdx2151) {
    _mdx2151_finalize(self_mdx2151);
    self_mdx2151 = NULL;
  }
  return FLAG_FALSE;
}

static void
self_destroy(void)
{
#if 0
  if (self_ym2151_c) {
    _ym2151_c_finalize(self_ym2151_c);
    self_ym2151_c = NULL;
  }
#endif
  if (self_pcm8) {
    _pcm8_finalize(self_pcm8);
    self_pcm8 = NULL;
  }
  if (self_mdxmml_ym2151) {
    _mdxmml_ym2151_finalize(self_mdxmml_ym2151);
    self_mdxmml_ym2151 = NULL;
  }
  if (self_mdx2151) {
    _mdx2151_finalize(self_mdx2151);
    self_mdx2151 = NULL;
  }
}

// ------------------------------------------------------------------------------------

static MDX_DATA *mdx;
static PDX_DATA *pdx;

static bool MDX_Open(const u8 *pmdxbuf,const u32 mdxbufsize,const u8 *ppdxbuf,const u32 pdxbufsize,u32 SampleRate,u32 SamplesPerFlame)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  mdx = NULL;
  pdx = NULL;
  
  if(!self_construct()){
    snprintf(pState->ErrorMessage,256,"Failed to create class instances.\n");
    return(false);
  }
  
  mdx=mdx_open_mdx(pmdxbuf,mdxbufsize);
  if(mdx==NULL){
    snprintf(pState->ErrorMessage,256,"MDX file load error.\n");
    return(false);
  }
  
  mdx->fm_wave_form        = 0;
  mdx->master_volume       = 127;
  mdx->fm_volume           = 127;
  mdx->pcm_volume          = 127;
  mdx->max_infinite_loops  = 2;
  mdx->fade_out_speed      = 5;
  
  mdx->is_output_to_stdout = FLAG_FALSE;
  mdx->is_use_fragment     = FLAG_TRUE;
  mdx->dsp_device          = (char*)NULL;
  mdx->dump_voice          = FLAG_FALSE;
  mdx->is_output_titles    = FLAG_TRUE;
  
  if ( mdx_get_voice_parameter( mdx ) != 0 ){
    snprintf(pState->ErrorMessage,256,"Func voice_parameter error.\n");
    mdx_close_mdx( mdx ); mdx=NULL;
    return(false);
  }
  
  /* load pdx data */
  if((ppdxbuf==NULL)||(pdxbufsize==0)){
    pdx=NULL;
    }else{
    pdx=_get_pdx(mdx, ppdxbuf,pdxbufsize);
  }
  
  mdx_parse_mml_ym2151_Start( mdx, pdx , SampleRate, SamplesPerFlame);
  
  return(true);
}

static const char* MDX_GetTitle(void)
{
  return(mdx->data_title);
}

static void MDX_Close(void)
{
  mdx_parse_mml_ym2151_End();
  LibSnd_DebugOut("End of MDX seq.\n");
  
  if(pdx!=NULL){
    mdx_close_pdx( pdx ); pdx=NULL;
  }
  if(mdx!=NULL){
    mdx_close_mdx( mdx ); mdx=NULL;
  }
  
  self_destroy();
}

// ------------------------------------------------------------------------------------

void error_end( char *msg ) {

  LibSnd_DebugOut( "%s\n", msg );
  SystemHalt();
}

static const char* GetPDXFilename(const char *pMDXFilename)
{
  const u32 headsize=1024;
  u8 head[headsize];
  
  {
    FILE *fp=fopen(pMDXFilename,"r");
    if(fp==NULL) return(NULL);
    fread(head,headsize,1,fp);
    fclose(fp);
  }
  
  static char pdxfn[256];
  strncpy(pdxfn,pMDXFilename,256);
  
  {
    u32 idx=0;
    u32 slashpos=(u32)-1;
    while(1){
      char ch=pdxfn[idx];
      if(ch==0) break;
      if((ch=='\\')||(ch=='/')) slashpos=idx;
      idx++;
    }
    if(slashpos==(u32)-1){
      pdxfn[0]=0;
      }else{
      pdxfn[slashpos+1]=0;
    }
  }
  
  for(u32 idx=0;idx<headsize-3;idx++){
    if((head[idx+0]==0x0d)&&(head[idx+1]==0x0a)&&(head[idx+2]==0x1a)){
      strcat(pdxfn,(char*)&head[idx+3]);
      return(pdxfn);
    }
  }
  
  return(NULL);
}

static void LibSndInt_ShowLicense(void)
{
  conout("mdxplay-20070206 version " VERSION_ID " [ " VERSION_TEXT1 " / " VERSION_TEXT2 " ]\n");
  conout("Made by NAGANO Daisuke <breeze.nagano@nifty.com> 1999-2006\n\n");
  conout("original:\n");
  conout("   X68k MXDRV music driver (c)1988-92 milk.,K.MAEKAWA, Missy.M, Yatsube\n");
  conout("ym2151.[ch]:\n");
  conout("   (c) 1997-2002 Jarek Burczynski (s0246@poczta.onet.pl, bujar@mame.net)\n");
  conout("   Some of the optimizing ideas by Tatsuyuki Satoh\n");
//  conout("freeverb is written by Jezar at Dreampoint, June 2000\n");

  conout("\n");
  conout("This software is under the GPL.\n");
  conout("This is free software; see the source for copying conditions.\n");
  conout("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n");
  conout("PARTICULAR PURPOSE.\n\n");
  
  conout("MDXTest for PSP by Moonlight.\n");
  conout("\n");
}

static u8 *pmdxbuf;
static u32 mdxbufsize;

static const char *pPDXFilename;

static bool LibSndInt_Open(const char *pFilename,const u32 TrackNum)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  pPDXFilename=GetPDXFilename(pFilename);
  
  {
    FILE *fp=fopen(pFilename,"r");
    fseek(fp,0,SEEK_END);
    mdxbufsize=ftell(fp);
    fseek(fp,0,SEEK_SET);
    
    pmdxbuf=(u8*)malloc(mdxbufsize);
    fread(pmdxbuf,mdxbufsize,1,fp);
    
    fclose(fp);
  }
  
  u8 *ppdxbuf=NULL;
  u32 pdxbufsize=0;
  
  if(pPDXFilename!=NULL){
    FILE *fp=fopen(pPDXFilename,"r");
    if(fp!=NULL){
      fseek(fp,0,SEEK_END);
      pdxbufsize=ftell(fp);
      fseek(fp,0,SEEK_SET);
      
      ppdxbuf=(u8*)malloc(pdxbufsize);
      fread(ppdxbuf,pdxbufsize,1,fp);
      
      fclose(fp);
    }
  }
  
  u32 SampleRate=44100;
  
  pState->SampleRate=SampleRate;
  pState->SamplesPerFlame=pState->SampleRate/120; // 120fps.
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=0;
  
  {
    if(MDX_Open(pmdxbuf,mdxbufsize,NULL,0,pState->SampleRate,pState->SamplesPerFlame)==false) return(false);
    while(mdx_parse_mml_ym2151_Loop(NULL,(double)pState->SamplesPerFlame/pState->SampleRate)==true){
      pState->TotalSec+=(float)pState->SamplesPerFlame/pState->SampleRate;
    }
    MDX_Close();
  }
  
  if(MDX_Open(pmdxbuf,mdxbufsize,ppdxbuf,pdxbufsize,pState->SampleRate,pState->SamplesPerFlame)==false) return(false);
  
  if(ppdxbuf!=NULL){
    free(ppdxbuf); ppdxbuf=NULL;
  }
  
  pState->pTitleW=EUC2Unicode_Convert(MDX_GetTitle());
  
  LibSnd_DebugOut("MDX decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Heavy);
}

static u32 LibSndInt_Update(u32 *pBufLR);

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(sec<pState->CurrentSec){
    mdx_init_track_work_area_ym2151();
    pState->CurrentSec=0;
  }
  
  while(pState->CurrentSec<sec){
    if(LibSndInt_Update(NULL)==0) break;
  }
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(mdx_parse_mml_ym2151_Loop(pBufLR,(double)pState->SamplesPerFlame/pState->SampleRate)==false){
    pState->isEnd=true;
    return(0);
  }
  
  pState->CurrentSec+=(float)pState->SamplesPerFlame/pState->SampleRate;
  
  return(pState->SamplesPerFlame);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  MDX_Close();
  
  if(pmdxbuf!=NULL){
    free(pmdxbuf); pmdxbuf=NULL;
  }
}

static int LibSndInt_GetInfoCount(void)
{
  return(2);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
  const char *pstr=NULL;
  
  switch(idx){
    case 0: {
      pstr=MDX_GetTitle();
    } break;
    case 1: {
      static char str[128];
      snprintf(str,128,"PDX: %s",str_GetFilenameFromFullPath(pPDXFilename));
      pstr=str;
    } break;
  }
  
  if(str_isEmpty(pstr)==false){
    wchar *pstrw=EUC2Unicode_Convert(pstr);
    Unicode_CopyNum(str,pstrw,len);
    if(pstrw!=NULL){
      safefree(pstrw); pstrw=NULL;
    }
    return(true);
  }
  
  return(false);
}

static void LibSndInt_ThreadSuspend(void)
{
}

static void LibSndInt_ThreadResume(void)
{
}

TLibSnd_Interface* LibSndMDX_GetInterface(void)
{
  static TLibSnd_Interface res={
    LibSndInt_ShowLicense,
    LibSndInt_Open,
    LibSndInt_GetPowerReq,
    LibSndInt_Seek,
    LibSndInt_Update,
    LibSndInt_Close,
    LibSndInt_GetInfoCount,
    LibSndInt_GetInfoStrUTF8,
    LibSndInt_GetInfoStrW,
    LibSndInt_ThreadSuspend,
    LibSndInt_ThreadResume,
    &LibSndInt_State,
  };
  return(&res);
}

