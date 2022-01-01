#pragma once

#include <pspuser.h>

// ���̃��C�u�����́A���������݂݂̂��ɂ����l�X�ȃP�A���X�~�X���������o���邽�߂ɍ��ꂽ�V���v���ȃ������}�l�[�W���ł��B
// �����I�ɂ́AsceIo�n�t�@�C���A�N�Z�X�֐��ƁAmalloc/free�����g���Ă��܂���B
// ���b�N����A�����b�N�܂ł����ł̓ǂݏ������������邱�Ƃɂ��A�X���b�v�t�@�C���i���z�������j���g����悤�ɂȂ��Ă��܂��B
// ���ʂ�malloc/free�ƕ��p���邱�Ƃ��ł��܂��B�i�����C�u�����Ȃǂ�malloc/free�g�p��j�Q���܂���j
// �i�X���b�v�t�@�C����L���ɂ��Ă��Ă��j���������ȏ�Ƀ��������g��Ȃ���Ώ������x�ቺ�́i���������Ɓj�y�����Ǝv���܂��B

// ���ӓ_�F�����_�ň�x�����g�p����Ă��Ȃ��̂ŁA�݁[��[���[���[�ɗ��߂ĉ������B
// ���ӓ_�F�������u���b�N�Ǘ��̈悪���\�������H���̂ŁAlibmem���ꂽ��N�����Ȃ��Ȃ����Ƃ��́APSP_HEAP_SIZE_KB���C�������Ȃ߂ɂ��Ă݂ĉ������B
// ���ӓ_�FMemRealloc�͂܂�����Ă��܂���B
// ���ӓ_�Fmalloc/free�G�~�����[�V�����n�֐��͂܂�����Ă��܂���B

// Version 0.1 prebeta 2011/09/23
// �Ƃ肠�����B

// Configuration defines -------------------------

// pspDebugScreenPrintf���g���Ƃ��͂��̍s��L���ɂ��ĉ������B�R�����g�A�E�g����ƕ��ʂ�printf���g�p���܂��B�iPSPLINK���Ŏg�p�j
//#define LIBMEM_printf pspDebugScreenPrintf
#define LIBMEM_printf conout

// �ő僁�����u���b�N�����w�肷��B���̐��ȏ�ɓ�����MemAlloc����Ɩ�������~���܂��B
#define LIBMEM_MemBlocksMax (1024)

// �g�p�p�r�������w�肷��BEMT_Normal�����ł��\���܂���B(EMT_Free, EMT_malloc, EMT_Count �͕K���w�肵�ĉ�����)
enum EMemType {EMT_Free=0,EMT_malloc,EMT_Normal,EMT_Count};
static const char *ppMemTypeStr[EMT_Count]={"Free","malloc","Normal"};

// �X���b�v�t�@�C���i���z�������j���g���B�g��Ȃ��Ƃ��Ƀ��C��������������Ȃ��Ȃ�Ɩ�������~���܂��B
#define LIBMEM_UseSwapFile

#ifdef LIBMEM_UseSwapFile

// �X���b�v�t�@�C���̍ő�T�C�Y(MBytes�P��)���w�肷��B��ꂽ�����������̃T�C�Y�𒴂���Ɩ�������~���܂��B
#define LIBMEM_SwapFileSizeMBytes (256)

// �X���b�v�t�@�C���̃t�@�C�������w�肵�܂��B�X���b�v�t�@�C�����g���Ƃ��͕K���w�肵�ĉ������B
#define LIBMEM_SwapFileFilename "swapfile.dat"

#endif // LIBMEM_UseSwapFile

// �`�F�b�N�T���ɂ�郁�������e�j������o����B
#define LIBMEM_UseCheckSumProtection

// �O��16bytes�ɓ���ȃf�[�^�����Ă����āA���������C�g�I�[�o�[���������o����B
#define LIBMEM_UseTermProtect

// ���������[�N�����o����B
#define LIBMEM_UseCheckMemoryLeak

// ���������̕\���t�H�[�}�b�g���w�肷��B
// 0 = Short.  (HMEM,       Size, Unlock/Locked,                      EMemType, SrcFN, SrcLine)
// 1 = Normal. (HMEM, Life, Size, Unlock/Locked, OnMem/SwapP, ChkSum, EMemType, SrcFN, SrcLine)
#define LIBMEM_ShowMemoryInfoFormat (1)

// �ُ팟�o���ɁA�S�Ẵ���������\�����Ă����~����B
#define LIBMEM_ShowAllocatedMemorysInfoOnHalt

// Mem???�n�֐��̌Ăяo��������\������B
#define LIBMEM_ShowFunctionTraceLog

// ------------------------------------------------------

#ifndef LIBMEM_printf
#define LIBMEM_printf printf
#endif

typedef u32 HMEM;

#define HMEM_NULL ((HMEM)0)

extern void MemInit(void);
extern void MemEnd(void);
extern void MemLeakCheck(EMemType MemType); // MemType��EMT_Free���w�肷��ƑS�Ẵ��������[�N���`�F�b�N���܂��B

#define MemAlloc(emt,size) _MemAlloc(__FILE__,__LINE__,emt,size)
extern HMEM _MemAlloc(const char *pFilename,const u32 LineNum,EMemType emt,u32 size);
extern void MemFree(HMEM hmem);

extern void* MemLockRW(HMEM hmem);
extern void MemUnlock(HMEM hmem);

extern void MemShowMemoryInfo(HMEM hmem);
extern void MemShowAllocatedMemorysInfo(EMemType MemType); // MemType��EMT_Free���w�肷��ƑS�Ẵ�������\�����܂��B

// ------------------------------------------------------

static inline void PrintFreeMem(void)
{
  const u32 maxsize=24*1024*1024;
  const u32 segsize=1*1024;
  const u32 count=maxsize/segsize;
  u32 *pptrs=(u32*)malloc(count*4);
  
  if(pptrs==NULL){
    LIBMEM_printf("PrintFreeMem: Investigation was interrupted. Very low free area.\n");
    SystemHalt();
  }
  
  u32 FreeMemSize=0;
  u32 MaxBlockSize=0;
  
  for(u32 idx=0;idx<count;idx++){
    u32 size=maxsize-(segsize*idx);
    pptrs[idx]=(u32)malloc(size);
    if(pptrs[idx]!=0){
      FreeMemSize+=size;
      if(MaxBlockSize<size) MaxBlockSize=size;
    }
  }
  
  LIBMEM_printf("FreeMem=%dkbyte (MaxBlockSize=%dkbyte)\n",FreeMemSize/1024,MaxBlockSize/1024);
  
  for(u32 idx=0;idx<count;idx++){
    if(pptrs[idx]!=0){
      free((void*)pptrs[idx]); pptrs[idx]=0;
    }
  }
  
  if(pptrs!=NULL){
    free(pptrs); pptrs=NULL;
  }
}

