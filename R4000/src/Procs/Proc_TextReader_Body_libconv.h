
enum ETextEncode {ETE_UTF8,ETE_UTF16BE,ETE_UTF16LE,ETE_SJIS,ETE_Count};

enum EReturnCode {ERC_CR,ERC_LF,ERC_CRLF,ERC_LFCR};

#define CharCR (0x0d)
#define CharLF (0x0a)

// -----------------------------------------------------------------------------------------

static ETextEncode libconv_DetectBOM(const u8 *pbuf,const u32 bufsize)
{
  if(3<=bufsize){
    u8 *pbom=(u8*)pbuf;
    if((pbom[0]==0xEF)&&(pbom[1]==0xBB)&&(pbom[2]==0xBF)) return(ETE_UTF8);
  }
  
  if(2<=bufsize){
    u16 bom=*(u16*)pbuf;
    if(bom==0xfffe) return(ETE_UTF16BE);
    if(bom==0xfeff) return(ETE_UTF16LE);
  }
  
  return(ETE_Count);
}

typedef struct {
  wchar wc;
  u32 ReadedCount;
} TlibconvResult;

static inline TlibconvResult libconv_Convert(const ETextEncode TextEncode,const u8 *pbuf)
{
  TlibconvResult res;
  
  switch(TextEncode){
    case ETE_UTF8: {
      u32 b0=pbuf[0],b1=pbuf[1],b2=pbuf[2];
      
      if(b0<0x80){
        res.wc=b0;
        res.ReadedCount=1;
        }else{
        if((b0&0xe0)==0xc0){ // 0b 110. ....
          res.wc=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
          res.ReadedCount=2;
          }else{
          if((b0&0xf0)==0xe0){ // 0b 1110 ....
            res.wc=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
            res.ReadedCount=3;
            pbuf+=3;
            }else{
            res.wc=0x00;
            res.ReadedCount=4;
          }
        }
      }
    } break;
    case ETE_UTF16BE: {
      u32 b0=pbuf[0],b1=pbuf[1];
      res.wc=(b0<<8)|(b1<<0);
      res.ReadedCount=2;
    } break;
    case ETE_UTF16LE: {
      u32 b0=pbuf[0],b1=pbuf[1];
      res.wc=(b0<<0)|(b1<<8);
      res.ReadedCount=2;
    } break;
    case ETE_SJIS: {
      TEUC2Unicode *ps2u=&EUC2Unicode;
      if(ps2u->Loaded==false){
        conout("Internal error: Not loaded EUC2Unicode table.\n");
        SystemHalt();
        }else{
        u32 b0=pbuf[0],b1=pbuf[1];
        if(ps2u->panktbl[b0]==true){
          if((0xa0<=b0)&&(b0<0xe0)) b0=0xff60+(b0-0xa0); // Hankaku Kana chars.
          res.wc=b0;
          res.ReadedCount=1;
          }else{
          if(b1==0){
            res.wc=0;
            res.ReadedCount=1;
            }else{
            res.wc=ps2u->ps2utbl[(b0<<8)|b1];
            res.ReadedCount=2;
          }
        }
      }
    } break;
    case ETE_Count: {
      conout("Internal error: ETE is ETE_Count?\n");
      SystemHalt();
    } break;
  }
  
  return(res);
}

// --------------------------------------------------------------------------

static u32 libconv_DetectTextEncode_ins_GetErrors(const ETextEncode TextEncode,const u8 *pbuf,const u32 bufsize,const u8 *pWidthsList)
{
  u32 errs=0;
  
  u32 idx=0;
  while(1){
    if(bufsize<=idx) break;
    TlibconvResult res=libconv_Convert(TextEncode,&pbuf[idx]);
    idx+=res.ReadedCount;
    if(0x20<=res.wc){
      u32 w=pWidthsList[res.wc];
      if(w==0) errs++;
    }
  }
  
  return(errs);
}

static ETextEncode libconv_DetectTextEncode(const u8 *pbuf,const u32 bufsize,const u8 *pWidthsList)
{
  ETextEncode res;
  
  res=libconv_DetectBOM(pbuf,bufsize);
  if(res!=ETE_Count) return(res);
  
  u32 chkbufsize=bufsize;
  if((64*1024)<chkbufsize) chkbufsize=64*1024;
  
  u32 Err_UTF8=libconv_DetectTextEncode_ins_GetErrors(ETE_UTF8,pbuf,chkbufsize,pWidthsList);
  conout("UTF-8 error rate: %dchars.\n",Err_UTF8);
  u32 Err_UTF16BE=libconv_DetectTextEncode_ins_GetErrors(ETE_UTF16BE,pbuf,chkbufsize,pWidthsList);
  conout("UTF-16BE error rate: %dchars.\n",Err_UTF16BE);
  u32 Err_UTF16LE=libconv_DetectTextEncode_ins_GetErrors(ETE_UTF16LE,pbuf,chkbufsize,pWidthsList);
  conout("UTF-16LE error rate: %dchars.\n",Err_UTF16LE);
  u32 Err_SJIS=libconv_DetectTextEncode_ins_GetErrors(ETE_SJIS,pbuf,chkbufsize,pWidthsList);
  conout("S-JIS error rate: %dchars.\n",Err_SJIS);
  
  res=ETE_SJIS;
  u32 Err=Err_SJIS;
  if(Err_UTF8<Err) res=ETE_UTF8;
  if(Err_UTF16BE<Err) res=ETE_UTF16BE;
  if(Err_UTF16LE<Err) res=ETE_UTF16LE;
  
  const char *pstr="";
  switch(res){
    case ETE_UTF8: pstr="UTF-8"; break;
    case ETE_UTF16BE: pstr="UTF-16BE"; break;
    case ETE_UTF16LE: pstr="UTF-16LE"; break;
    case ETE_SJIS: pstr="S-JIS"; break;
    case ETE_Count: {
      conout("Internal error: ETE is ETE_Count?\n");
      SystemHalt();
    } break;
  }
  conout("Text encode %s detected.\n",pstr);
  
  return(res);
}

// ----------------------------------------------------------------------------------

static EReturnCode libconv_DetectReturnCode(const ETextEncode TextEncode,const u8 *pbuf,const u32 bufsize)
{
  EReturnCode res=ERC_CRLF;
  
  u32 idx=0;
  while(1){
    if(bufsize<=idx) break;
    TlibconvResult lcres=libconv_Convert(TextEncode,&pbuf[idx]);
    idx+=lcres.ReadedCount;
    if(lcres.wc==CharCR){
      TlibconvResult lcres=libconv_Convert(TextEncode,&pbuf[idx]);
      idx+=lcres.ReadedCount;
      if(lcres.wc==CharLF){
        res=ERC_CRLF;
        break;
        }else{
        res=ERC_CR;
        break;
      }
    }
    if(lcres.wc==CharLF){
      TlibconvResult lcres=libconv_Convert(TextEncode,&pbuf[idx]);
      idx+=lcres.ReadedCount;
      if(lcres.wc==CharCR){
        res=ERC_LFCR;
        break;
        }else{
        res=ERC_LF;
        break;
      }
    }
  }
  
  const char *pstr="";
  switch(res){
    case ERC_CR: pstr="CR"; break;
    case ERC_LF: pstr="LF"; break;
    case ERC_CRLF: pstr="CRLF"; break;
    case ERC_LFCR: pstr="LFCR"; break;
  }
  conout("Return code %s detected.\n",pstr);
  
  return(res);
}

