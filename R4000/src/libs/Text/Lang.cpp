
#include <stdio.h>
#include <stdlib.h>

#include <pspuser.h>

#include "common.h"
#include "strtool.h"
#include "unicode.h"
#include "GlobalINI.h"

const char* GetLangStr(const char *pstre,const char *pstrj)
{
  const char *pstr=pstre;
  
  switch(GlobalINI.System.Language){
    case CP0_JPN: {
      if(str_isEmpty(pstrj)==false) pstr=pstrj;
    } break;
    case CP1_ENG: {
    } break;
  }
  
  return(pstr);
}


