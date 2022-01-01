
#include <string.h>

typedef char* AP4_String;

#define AP4_String_c_str(x) (x)
#define AP4_String_length(x) (strlen(x))
//#define AP4_String_AllocCopy(psrc) str_AllocateCopy(&MM_DLLSound,psrc)
#define AP4_String_AllocCopy(psrc) NULL; conout(0,"not-supported C++ strings."); while(1);
