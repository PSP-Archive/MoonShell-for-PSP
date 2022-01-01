
static void SeekFile_SetFilename(s32 Vector)
{
  u32 FilenamesCount=0;
  u32 FilenamesIndex=(u32)-1;
  
  {
    SceUID dfd=sceIoDopen(Proc_ImageView_ImagePath);
    if(dfd<0){
      conout("sceIoDopen error. SEC=0x%08x.\n",dfd);
      SystemHalt();
    }
    
    SceIoDirent dir;
    MemSet8CPU(0,&dir,sizeof(SceIoDirent));
    
    while(1){
      int dreadres=sceIoDread(dfd,&dir);
      if(dreadres==0) break;
      if(dreadres<0){
        conout("sceIoDread error. SEC=0x%08x.\n",dreadres);
        SystemHalt();
      }
      if(FIO_SO_ISDIR(dir.d_stat.st_attr)){
      }
      if(FIO_SO_ISREG(dir.d_stat.st_attr)){
        if(LibImg_isSupportFile(dir.d_name)==true){
          if(isStrEqual(dir.d_name,Proc_ImageView_ImageFilename)==true) FilenamesIndex=FilenamesCount;
          FilenamesCount++;
        }
      }
    }
    
    sceIoDclose(dfd);
  }
  
  if(FilenamesCount==0){
    conout("Fatal error: Not found enum image files.\n");
    SystemHalt();
  }
  
  if(FilenamesIndex==(u32)-1){
    conout("Fatal error: Not found current image file.\n");
    SystemHalt();
  }
  
  u32 FilenamesTarget=FilenamesIndex;
  
  if(Vector==-1){
    if(FilenamesTarget==0){
      FilenamesTarget=FilenamesCount-1;
      }else{
      FilenamesTarget--;
    }
  }
  if(Vector==+1){
    if(FilenamesTarget==(FilenamesCount-1)){
      FilenamesTarget=0;
      }else{
      FilenamesTarget++;
    }
  }
  
  FilenamesCount=0;
  FilenamesIndex=(u32)-1;
  
  {
    SceUID dfd=sceIoDopen(Proc_ImageView_ImagePath);
    if(dfd<0){
      conout("sceIoDopen error. SEC=0x%08x.\n",dfd);
      SystemHalt();
    }
    
    SceIoDirent dir;
    MemSet8CPU(0,&dir,sizeof(SceIoDirent));
    
    while(1){
      int dreadres=sceIoDread(dfd,&dir);
      if(dreadres==0) break;
      if(dreadres<0){
        conout("sceIoDread error. SEC=0x%08x.\n",dreadres);
        SystemHalt();
      }
      if(FIO_SO_ISDIR(dir.d_stat.st_attr)){
      }
      if(FIO_SO_ISREG(dir.d_stat.st_attr)){
        if(LibImg_isSupportFile(dir.d_name)==true){
          if(FilenamesTarget==FilenamesCount) StrCopyNum(dir.d_name,Proc_ImageView_ImageFilename,256);
          FilenamesCount++;
        }
      }
    }
    
    sceIoDclose(dfd);
  }
}

