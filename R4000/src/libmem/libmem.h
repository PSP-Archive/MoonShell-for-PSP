#pragma once

#include <pspuser.h>

// このライブラリは、メモリ絡みのみつけにくい様々なケアレスミスを自動検出するために作られたシンプルなメモリマネージャです。
// 内部的には、sceIo系ファイルアクセス関数と、malloc/freeしか使っていません。
// ロックからアンロックまでだけでの読み書きを強制することにより、スワップファイル（仮想メモリ）も使えるようになっています。
// 普通のmalloc/freeと併用することができます。（他ライブラリなどのmalloc/free使用を阻害しません）
// （スワップファイルを有効にしていても）実メモリ以上にメモリを使わなければ処理速度低下は（多分きっと）軽微だと思います。

// 注意点：現時点で一度も実使用されていないので、みーるーだーけーに留めて下さい。
// 注意点：メモリブロック管理領域が結構メモリ食うので、libmem入れたら起動しなくなったときは、PSP_HEAP_SIZE_KBを気持ち少なめにしてみて下さい。
// 注意点：MemReallocはまだ作っていません。
// 注意点：malloc/freeエミュレーション系関数はまだ作っていません。

// Version 0.1 prebeta 2011/09/23
// とりあえず。

// Configuration defines -------------------------

// pspDebugScreenPrintfを使うときはこの行を有効にして下さい。コメントアウトすると普通のprintfを使用します。（PSPLINK等で使用）
//#define LIBMEM_printf pspDebugScreenPrintf
#define LIBMEM_printf conout

// 最大メモリブロック数を指定する。この数以上に同時にMemAllocすると無条件停止します。
#define LIBMEM_MemBlocksMax (1024)

// 使用用途属性を指定する。EMT_Normalだけでも構いません。(EMT_Free, EMT_malloc, EMT_Count は必ず指定して下さい)
enum EMemType {EMT_Free=0,EMT_malloc,EMT_Normal,EMT_Count};
static const char *ppMemTypeStr[EMT_Count]={"Free","malloc","Normal"};

// スワップファイル（仮想メモリ）を使う。使わないときにメインメモリが足りなくなると無条件停止します。
#define LIBMEM_UseSwapFile

#ifdef LIBMEM_UseSwapFile

// スワップファイルの最大サイズ(MBytes単位)を指定する。溢れたメモリがこのサイズを超えると無条件停止します。
#define LIBMEM_SwapFileSizeMBytes (256)

// スワップファイルのファイル名を指定します。スワップファイルを使うときは必ず指定して下さい。
#define LIBMEM_SwapFileFilename "swapfile.dat"

#endif // LIBMEM_UseSwapFile

// チェックサムによるメモリ内容破壊を検出する。
#define LIBMEM_UseCheckSumProtection

// 前後16bytesに特殊なデータを入れておいて、メモリライトオーバーランを検出する。
#define LIBMEM_UseTermProtect

// メモリリークを検出する。
#define LIBMEM_UseCheckMemoryLeak

// メモリ情報の表示フォーマットを指定する。
// 0 = Short.  (HMEM,       Size, Unlock/Locked,                      EMemType, SrcFN, SrcLine)
// 1 = Normal. (HMEM, Life, Size, Unlock/Locked, OnMem/SwapP, ChkSum, EMemType, SrcFN, SrcLine)
#define LIBMEM_ShowMemoryInfoFormat (1)

// 異常検出時に、全てのメモリ情報を表示してから停止する。
#define LIBMEM_ShowAllocatedMemorysInfoOnHalt

// Mem???系関数の呼び出し履歴を表示する。
#define LIBMEM_ShowFunctionTraceLog

// ------------------------------------------------------

#ifndef LIBMEM_printf
#define LIBMEM_printf printf
#endif

typedef u32 HMEM;

#define HMEM_NULL ((HMEM)0)

extern void MemInit(void);
extern void MemEnd(void);
extern void MemLeakCheck(EMemType MemType); // MemTypeにEMT_Freeを指定すると全てのメモリリークをチェックします。

#define MemAlloc(emt,size) _MemAlloc(__FILE__,__LINE__,emt,size)
extern HMEM _MemAlloc(const char *pFilename,const u32 LineNum,EMemType emt,u32 size);
extern void MemFree(HMEM hmem);

extern void* MemLockRW(HMEM hmem);
extern void MemUnlock(HMEM hmem);

extern void MemShowMemoryInfo(HMEM hmem);
extern void MemShowAllocatedMemorysInfo(EMemType MemType); // MemTypeにEMT_Freeを指定すると全てのメモリを表示します。

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

