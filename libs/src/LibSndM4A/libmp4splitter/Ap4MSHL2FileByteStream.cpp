/*****************************************************************
|
|      File Byte Stream
|
|      (c) 2001-2002 Gilles Boccon-Gibod
|
|
 ****************************************************************/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <pspuser.h>

#include "Core/Ap4FileByteStream.h"

static FILE *m_File;

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream
+---------------------------------------------------------------------*/
class AP4_MSHL2FileByteStream: public AP4_ByteStream
{
public:
    // methods
    AP4_MSHL2FileByteStream(AP4_ByteStream*          delegator,
                           void *pFileHandle);
    ~AP4_MSHL2FileByteStream();

    // AP4_ByteStream methods
    AP4_Result Read(void*    buffer, 
                   AP4_Size  bytesToRead, 
                   AP4_Size* bytesRead);
    AP4_Result Seek(AP4_Offset offset);
    AP4_Result Tell(AP4_Offset& offset);
    AP4_Result GetSize(AP4_Size& size);
    
    // AP4_Referenceable methods
    void AddReference();
    void Release();

private:
    // members
    AP4_ByteStream* m_Delegator;
    AP4_Cardinal    m_ReferenceCount;
    
    u32 FileSize;
    
    u8 *pbuf;
    u32 bufpos,bufsize;
};

static const u32 bufmaxsize=4*1024;

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream::AP4_MSHL2FileByteStream
+---------------------------------------------------------------------*/
AP4_MSHL2FileByteStream::AP4_MSHL2FileByteStream(
    AP4_ByteStream  *        delegator,
    void *pFileHandle) :
    m_Delegator(delegator),
    m_ReferenceCount(1)
{
  m_File=(FILE*)pFileHandle;
  
  u32 ofs=ftell(m_File);
  fseek(m_File,0,SEEK_END);
  FileSize=ftell(m_File);
  fseek(m_File,ofs,SEEK_SET);
  
  pbuf=(u8*)malloc(bufmaxsize);
  bufpos=0;
  bufsize=0;
}

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream::~AP4_MSHL2FileByteStream
+---------------------------------------------------------------------*/
AP4_MSHL2FileByteStream::~AP4_MSHL2FileByteStream()
{
  m_File=NULL;
  
  if(pbuf!=NULL){
    free(pbuf); pbuf=NULL;
  }
  bufpos=0;
}

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream::AddReference
+---------------------------------------------------------------------*/
void
AP4_MSHL2FileByteStream::AddReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream::Release
+---------------------------------------------------------------------*/
void
AP4_MSHL2FileByteStream::Release()
{
    if (--m_ReferenceCount == 0) {
        delete m_Delegator;
    }
}

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream::Read
+---------------------------------------------------------------------*/
AP4_Result
AP4_MSHL2FileByteStream::Read(void*    buffer, 
                            AP4_Size  bytesToRead, 
                            AP4_Size* bytesRead)
{
/*
  u32 a=fread(buffer,1,bytesToRead,m_File);
  if(bytesRead) *bytesRead=a;
  return AP4_SUCCESS;
*/
  
/*
  if(bytesToRead<=8){
    conout(".");
    }else{
    conout("%d(%d/%d) ",bytesToRead,bufpos,bufsize);
  }
*/
  
  if(bytesToRead==0){
    if(bytesRead) *bytesRead=bytesToRead;
    return AP4_SUCCESS;
  }
  
  u32 curpos=ftell(m_File)-bufsize+bufpos;
  u32 lastsize=FileSize-curpos;
  
  if(lastsize==0) return AP4_ERROR_EOS;
  
  if(lastsize<bytesToRead) bytesToRead=lastsize;
  
  if(bytesRead) *bytesRead=bytesToRead;
  
  u8 *pdstbuf=(u8*)buffer;
  
  while(1){
    u32 lastbufsize=bufsize-bufpos;
    u32 readsize=bytesToRead;
    if(lastbufsize<readsize) readsize=lastbufsize;
    if(readsize!=0){
      memcpy(pdstbuf,&pbuf[bufpos],readsize);
      bufpos+=readsize;
      pdstbuf+=readsize;
      bytesToRead-=readsize;
      if(bytesToRead==0) return AP4_SUCCESS;
    }
    bufsize=fread(pbuf,1,bufmaxsize,m_File);
    bufpos=0;
  }
}

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream::Seek
+---------------------------------------------------------------------*/
AP4_Result
AP4_MSHL2FileByteStream::Seek(AP4_Offset offset)
{
  s32 curofs = ftell(m_File)-bufsize+bufpos;
  if(curofs==offset) return AP4_SUCCESS;
  
  s32 move=offset-curofs;
  if(0<move){
    if(move<=(bufsize-bufpos)){
      bufpos+=move;
      return AP4_SUCCESS;
    }
  }
  
//  conout("FSEEK: %d -> %dbytes. (%d/%d)\n",curofs,offset,bufpos,bufsize);
  
  size_t result = fseek(m_File, offset&~3, SEEK_SET);
  
  bufpos=offset&3;
  if(bufpos==0){
    bufsize=0;
    }else{
    bufsize=fread(pbuf,1,bufmaxsize,m_File);
    if(bufsize<bufpos) bufpos=bufsize;
  }
  
  return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream::Tell
+---------------------------------------------------------------------*/
AP4_Result
AP4_MSHL2FileByteStream::Tell(AP4_Offset& offset)
{
    offset = ftell(m_File)-bufsize+bufpos;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_MSHL2FileByteStream::GetSize
+---------------------------------------------------------------------*/
AP4_Result
AP4_MSHL2FileByteStream::GetSize(AP4_Size& size)
{
    size = FileSize;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_FileByteStream::AP4_FileByteStream
+---------------------------------------------------------------------*/
AP4_FileByteStream::AP4_FileByteStream(void *pFileHandle)
{
    m_Delegate = new AP4_MSHL2FileByteStream(this, pFileHandle);
}

extern void AP4_MSHL2FileByteStream_SetFileHandle(void *pFileHandle);
void AP4_MSHL2FileByteStream_SetFileHandle(void *pFileHandle)
{
  if(m_File==NULL){
    conout("Internal error: mp4splitter not opened. (m_File==NULL)\n");
    SystemHalt();
  }
  m_File=(FILE*)pFileHandle;
}











