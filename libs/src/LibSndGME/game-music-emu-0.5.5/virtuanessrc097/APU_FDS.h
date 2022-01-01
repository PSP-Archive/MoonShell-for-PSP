//////////////////////////////////////////////////////////////////////////
//                                                                      //
//      FDS plugin                                                      //
//                                                           Norix      //
//                                               written     2001/09/18 //
//                                               last modify ----/--/-- //
//////////////////////////////////////////////////////////////////////////
#ifndef  __FDSPLUGIN_INCLUDED__
#define  __FDSPLUGIN_INCLUDED__

#include <pspuser.h>

typedef float FLOAT;
typedef u8 BYTE;
typedef u16 WORD;
typedef s32 INT;

#define TRUE true
#define FALSE false
#define BOOL bool

class APU_FDS
{
public:
  APU_FDS();
  ~APU_FDS();

  void  Write( u16 addr, u8 data );
  BYTE  Read ( u16 addr );
//  INT  Process( void );
  void ProcessBuffer(u32 *pBufLR32,u32 SamplesCount,s32 GainShift14);
  void ShowState(void);
  bool GetEnabled(void);

protected:
private:
};

extern APU_FDS *pVNES_APU_FDS;

#endif  // !__FDSPLUGIN_INCLUDED__
