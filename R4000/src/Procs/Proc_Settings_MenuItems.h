
static void SetupMenuItems(CMenu *pm)
{
  {
    TProcState_Global *ps=&ProcState.Global;
    CMenuGroup *pmg=pm->CreateGroup(&ps->ItemsIndex,GetLangStr("Global","全体設定"));
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("CPUFreq for normal","CPU速度（通常）"),"",(u8*)&ps->CPUFreqForNormal);
      pmgi->CreateSet(GetLangStr("66MHz",""),GetLangStr("Very slow. (Not recommended)","とても遅い（非推奨）"),TProcState_Global::ECF_66);
      pmgi->CreateSet(GetLangStr("100MHz",""),GetLangStr("",""),TProcState_Global::ECF_100);
      pmgi->CreateSet(GetLangStr("111MHz",""),GetLangStr("",""),TProcState_Global::ECF_111);
      pmgi->CreateSet(GetLangStr("133MHz",""),GetLangStr("",""),TProcState_Global::ECF_133);
      pmgi->CreateSet(GetLangStr("166MHz",""),GetLangStr("",""),TProcState_Global::ECF_166);
      pmgi->CreateSet(GetLangStr("200MHz",""),GetLangStr("",""),TProcState_Global::ECF_200);
      pmgi->CreateSet(GetLangStr("222MHz",""),GetLangStr("",""),TProcState_Global::ECF_222);
      pmgi->CreateSet(GetLangStr("233MHz",""),GetLangStr("",""),TProcState_Global::ECF_233);
      pmgi->CreateSet(GetLangStr("266MHz",""),GetLangStr("",""),TProcState_Global::ECF_266);
      pmgi->CreateSet(GetLangStr("300MHz",""),GetLangStr("",""),TProcState_Global::ECF_300);
      pmgi->CreateSet(GetLangStr("333MHz",""),GetLangStr("",""),TProcState_Global::ECF_333);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("CPUFreq for hold","CPU速度（HOLD時）"),"",(u8*)&ps->CPUFreqForHold);
      pmgi->CreateSet(GetLangStr("33MHz",""),GetLangStr("Very slow. (Not recommended)","とても遅い（非推奨）"),TProcState_Global::ECF_33);
      pmgi->CreateSet(GetLangStr("66MHz",""),GetLangStr("",""),TProcState_Global::ECF_66);
      pmgi->CreateSet(GetLangStr("100MHz",""),GetLangStr("",""),TProcState_Global::ECF_100);
      pmgi->CreateSet(GetLangStr("111MHz",""),GetLangStr("",""),TProcState_Global::ECF_111);
      pmgi->CreateSet(GetLangStr("133MHz",""),GetLangStr("",""),TProcState_Global::ECF_133);
      pmgi->CreateSet(GetLangStr("166MHz",""),GetLangStr("",""),TProcState_Global::ECF_166);
      pmgi->CreateSet(GetLangStr("200MHz",""),GetLangStr("",""),TProcState_Global::ECF_200);
      pmgi->CreateSet(GetLangStr("222MHz",""),GetLangStr("",""),TProcState_Global::ECF_222);
      pmgi->CreateSet(GetLangStr("233MHz",""),GetLangStr("",""),TProcState_Global::ECF_233);
      pmgi->CreateSet(GetLangStr("266MHz",""),GetLangStr("",""),TProcState_Global::ECF_266);
      pmgi->CreateSet(GetLangStr("300MHz",""),GetLangStr("",""),TProcState_Global::ECF_300);
      pmgi->CreateSet(GetLangStr("333MHz",""),GetLangStr("",""),TProcState_Global::ECF_333);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Sound effect","効果音"),"",(u8*)&ps->UseSE);
      pmgi->CreateSet(GetLangStr("OFF",""),GetLangStr("",""),0);
      pmgi->CreateSet(GetLangStr("ON",""),GetLangStr("",""),1);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Equalizer (Bass)","イコライザ（低音）"),"",(u8*)&ps->EQ_BassLevel);
      pmgi->CreateSet(GetLangStr("0",""),GetLangStr("",""),0);
      pmgi->CreateSet(GetLangStr("+1",""),GetLangStr("",""),+1);
      pmgi->CreateSet(GetLangStr("+2",""),GetLangStr("",""),+2);
      pmgi->CreateSet(GetLangStr("+3",""),GetLangStr("",""),+3);
      pmgi->CreateSet(GetLangStr("+4",""),GetLangStr("",""),+4);
      pmgi->CreateSet(GetLangStr("+5",""),GetLangStr("",""),+5);
      pmgi->CreateSet(GetLangStr("+6",""),GetLangStr("",""),+6);
      pmgi->CreateSet(GetLangStr("+7",""),GetLangStr("",""),+7);
      pmgi->CreateSet(GetLangStr("+8",""),GetLangStr("",""),+8);
      pmgi->CreateSet(GetLangStr("+9",""),GetLangStr("",""),+9);
      pmgi->CreateSet(GetLangStr("+10",""),GetLangStr("",""),+10);
      pmgi->CreateSet(GetLangStr("+11",""),GetLangStr("",""),+11);
      pmgi->CreateSet(GetLangStr("+12",""),GetLangStr("",""),+12);
      pmgi->CreateSet(GetLangStr("+13",""),GetLangStr("",""),+13);
      pmgi->CreateSet(GetLangStr("+14",""),GetLangStr("",""),+14);
      pmgi->CreateSet(GetLangStr("+15",""),GetLangStr("",""),+15);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Equalizer (Treble)","イコライザ（高音）"),"",(u8*)&ps->EQ_TrebleLevel);
      pmgi->CreateSet(GetLangStr("-15",""),GetLangStr("",""),-15);
      pmgi->CreateSet(GetLangStr("-14",""),GetLangStr("",""),-14);
      pmgi->CreateSet(GetLangStr("-13",""),GetLangStr("",""),-13);
      pmgi->CreateSet(GetLangStr("-12",""),GetLangStr("",""),-12);
      pmgi->CreateSet(GetLangStr("-11",""),GetLangStr("",""),-11);
      pmgi->CreateSet(GetLangStr("-10",""),GetLangStr("",""),-10);
      pmgi->CreateSet(GetLangStr("-9",""),GetLangStr("",""),-9);
      pmgi->CreateSet(GetLangStr("-8",""),GetLangStr("",""),-8);
      pmgi->CreateSet(GetLangStr("-7",""),GetLangStr("",""),-7);
      pmgi->CreateSet(GetLangStr("-6",""),GetLangStr("",""),-6);
      pmgi->CreateSet(GetLangStr("-5",""),GetLangStr("",""),-5);
      pmgi->CreateSet(GetLangStr("-4",""),GetLangStr("",""),-4);
      pmgi->CreateSet(GetLangStr("-3",""),GetLangStr("",""),-3);
      pmgi->CreateSet(GetLangStr("-2",""),GetLangStr("",""),-2);
      pmgi->CreateSet(GetLangStr("-1",""),GetLangStr("",""),-1);
      pmgi->CreateSet(GetLangStr("0",""),GetLangStr("",""),0);
      pmgi->SetDefault();
    }
    pmg->SetDefault();
  }
  
  {
    TProcState_FileList *ps=&ProcState.FileList;
    CMenuGroup *pmg=pm->CreateGroup(&ps->ItemsIndex,GetLangStr("FileList",""));
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Filename font size","ファイル名サイズ"),"",(u8*)&ps->FilenameFontSize);
      pmgi->CreateSet(GetLangStr("a12A","A12あ"),GetLangStr("Very small","とても小さい"),12,12);
      pmgi->CreateSet(GetLangStr("a14A","A14あ"),GetLangStr("Small","小さい"),14,14);
      pmgi->CreateSet(GetLangStr("a16A","A16あ"),GetLangStr("Normal","標準"),16,16);
      pmgi->CreateSet(GetLangStr("a20A","A20あ"),GetLangStr("Large","大きい"),20,20);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Show file icon","ファイルアイコン"),"",(u8*)&ps->ShowFileIcon);
      pmgi->CreateSet(GetLangStr("OFF",""),GetLangStr("Filename only.","ファイル名のみ表示"),0);
      pmgi->CreateSet(GetLangStr("ON",""),GetLangStr("Icon and filename.","アイコンとファイル名を表示"),1);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Number of lines","ファイル名行数"),"",(u8*)&ps->NumberOfLines);
      pmgi->CreateSet(GetLangStr("1",""),GetLangStr("A long file name cuts.","長いファイル名は切り捨てる"),1);
      pmgi->CreateSet(GetLangStr("2",""),GetLangStr("A long file name is divided.","長いファイル名は二行に分ける"),2);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("X button assignment","×ボタン割り当て"),"",(u8*)&ps->BButtonAssignment);
      pmgi->CreateSet(GetLangStr("Folder-up","フォルダ戻り"),GetLangStr("Exit folder.","一つ前のフォルダに戻る"),TProcState_FileList::EBBA_FolderUp);
      pmgi->CreateSet(GetLangStr("Music-stop","音楽停止"),GetLangStr("Music stop.","再生中の音楽を停止する"),TProcState_FileList::EBBA_MusicStop);
      pmgi->CreateSet(GetLangStr("Auto","自動"),GetLangStr("Exit folder or music stop.","再生中なら音楽停止。又はフォルダから出る"),TProcState_FileList::EBBA_Auto);
      pmgi->SetDefault();
    }
    pmg->SetDefault();
  }
  
  {
    TProcState_PlayTab *ps=&ProcState.PlayTab;
    CMenuGroup *pmg=pm->CreateGroup(&ps->ItemsIndex,GetLangStr("PlayTab",""));
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Filename font size (Req.reboot)","ファイル名サイズ（要再起動）"),"",(u8*)&ps->FilenameFontSize);
      pmgi->CreateSet(GetLangStr("a12A","A12あ"),GetLangStr("Very small","とても小さい"),12,12);
      pmgi->CreateSet(GetLangStr("a14A","A14あ"),GetLangStr("Small","小さい"),14,14);
      pmgi->CreateSet(GetLangStr("a16A","A16あ"),GetLangStr("Normal","標準"),16,16);
      pmgi->CreateSet(GetLangStr("a20A","A20あ"),GetLangStr("Large","大きい"),20,20);
      pmgi->CreateSet(GetLangStr("a24A","A24あ"),GetLangStr("Very large","とても大きい"),24,24);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Number of lines","ファイル名行数"),"",(u8*)&ps->NumberOfLines);
      pmgi->CreateSet(GetLangStr("1",""),GetLangStr("Current + Next","現在曲"),1);
      pmgi->CreateSet(GetLangStr("2",""),GetLangStr("Current + Next","現在曲＋次曲"),2);
      pmgi->CreateSet(GetLangStr("3",""),GetLangStr("Back + Current + Next","前曲＋現在曲＋次曲"),3);
      pmgi->CreateSet(GetLangStr("4",""),GetLangStr("Back + Current + Next + Next","前曲＋現在曲＋次曲＋次曲"),4);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Music info font size (Req.reboot)","曲情報サイズ（要再起動）"),"",(u8*)&ps->MusicInfoFontSize);
      pmgi->CreateSet(GetLangStr("a12A","A12あ"),GetLangStr("Very small","とても小さい"),12,12);
      pmgi->CreateSet(GetLangStr("a14A","A14あ"),GetLangStr("Small","小さい"),14,14);
      pmgi->CreateSet(GetLangStr("a16A","A16あ"),GetLangStr("Normal","標準"),16,16);
      pmgi->CreateSet(GetLangStr("a20A","A20あ"),GetLangStr("Large","大きい"),20,20);
      pmgi->CreateSet(GetLangStr("a24A","A24あ"),GetLangStr("Very large","とても大きい"),24,24);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Show internal info.","曲データ内部情報を表示"),"",(u8*)&ps->AddInternalInfoToMusicInfo);
      pmgi->CreateSet(GetLangStr("Hidden","隠す"),GetLangStr("Music info only.","基本曲情報のみ表示"),0);
      pmgi->CreateSet(GetLangStr("Show","表示"),GetLangStr("Add to music info.","内部情報を曲情報表示に追加"),1);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Play list mode","プレイリスト演奏モード"),"",(u8*)&ps->PlayListMode);
      pmgi->CreateSet(GetLangStr("OneStop","一曲停止"),GetLangStr("When music finishes, music stops.","一曲再生後に停止"),TProcState_PlayTab::EPLM_OneStop);
      pmgi->CreateSet(GetLangStr("OneRepeat","一曲リピート"),GetLangStr("Repeat play of the same music.","同じ曲を再生し続ける"),TProcState_PlayTab::EPLM_OneRepeat);
      pmgi->CreateSet(GetLangStr("AllStop","全曲停止"),GetLangStr("It stops, after playing all the music.","全ての曲を再生後に停止"),TProcState_PlayTab::EPLM_AllStop);
      pmgi->CreateSet(GetLangStr("AllRepeat","全曲リピート"),GetLangStr("Playing all the music is continued.","全ての曲を再生し続ける"),TProcState_PlayTab::EPLM_AllRepeat);
      pmgi->CreateSet(GetLangStr("Shuffle","シャッフル再生"),GetLangStr("A row is shuffled and it continued.","曲順をシャッフルして再生し続ける"),TProcState_PlayTab::EPLM_Shuffle);
      pmgi->SetDefault();
    }
    pmg->SetDefault();
  }
  
  {
    TProcState_Image *ps=&ProcState.Image;
    CMenuGroup *pmg=pm->CreateGroup(&ps->ItemsIndex,GetLangStr("Image",""));
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Auto jpeg fitting.","JPEGファイル自動縮小"),"",(u8*)&ps->AutoJpegFitting);
      pmgi->CreateSet(GetLangStr("ON",""),GetLangStr("An extremely big JPEG file is reduced.","大きすぎる画像は自動調整する"),1);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Show image info window.","画像情報ウィンドウ"),"",(u8*)&ps->ShowInfoWindow);
      pmgi->CreateSet(GetLangStr("Hidden","隠す"),GetLangStr("Hide window","ウィンドウを隠す"),0);
      pmgi->CreateSet(GetLangStr("Show","表示"),GetLangStr("Size, Pos, Ratio, FPS, Music.","サイズ、位置、拡大率、FPS、音楽情報"),1);
      pmgi->SetDefault();
    }
    
    pmg->SetDefault();
  }
  
  {
    TProcState_Text *ps=&ProcState.Text;
    CMenuGroup *pmg=pm->CreateGroup(&ps->ItemsIndex,GetLangStr("Text",""));
    {
      static u8 value=0;
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("No setting item","設定項目がありません。"),"",(u8*)&value);
      pmgi->CreateSet(GetLangStr("X",""),GetLangStr("",""),0);
      pmgi->SetDefault();
    }
    pmg->SetDefault();
  }
  
  {
    TProcState_SClock *ps=&ProcState.SClock;
    CMenuGroup *pmg=pm->CreateGroup(&ps->ItemsIndex,GetLangStr("ScrClock","時計"));
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Timeout sec.","タイムアウト時間（秒）"),"",(u8*)&ps->TimeoutSec);
      pmgi->CreateSet(GetLangStr("OFF",""),GetLangStr("Not use screen clock.","時計を使わない"),0);
      pmgi->CreateSet(GetLangStr("3",""),GetLangStr("Starts in 3 seconds.","3秒で表示する。"),3);
      pmgi->CreateSet(GetLangStr("10",""),GetLangStr("Starts in 10 seconds.","10秒で表示する。"),10);
      pmgi->CreateSet(GetLangStr("30",""),GetLangStr("Starts in 30 seconds.","30秒で表示する。"),30);
      pmgi->CreateSet(GetLangStr("60",""),GetLangStr("Starts in 1 minutes.","1分で表示する。"),60);
      pmgi->CreateSet(GetLangStr("120",""),GetLangStr("Starts in 2 minutes.","2分で表示する。"),120);
      pmgi->CreateSet(GetLangStr("180",""),GetLangStr("Starts in 3 minutes.","3分で表示する。"),180);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("BG scroll speed.","背景スクロール速度"),"",(u8*)&ps->ScrollSpeed);
      pmgi->CreateSet(GetLangStr("Stop","停止"),GetLangStr("Not use BG scroll.","停止"),TProcState_SClock::ESS_Stop);
      pmgi->CreateSet(GetLangStr("Slow","遅い"),GetLangStr("Late scrolling.","遅い"),TProcState_SClock::ESS_Slow);
      pmgi->CreateSet(GetLangStr("Normal","標準"),GetLangStr("Ordinary scrolling.","標準"),TProcState_SClock::ESS_Normal);
      pmgi->CreateSet(GetLangStr("Fast","速い"),GetLangStr("Quick scrolling.","速い"),TProcState_SClock::ESS_Fast);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Transparent BG alpha.","背景透過率"),"",(u8*)&ps->BGAlpha);
      pmgi->CreateSet(GetLangStr("OFF",""),GetLangStr("Not transparent. (Recommend)","透過しない（推奨）"),0xff-0x00);
      pmgi->CreateSet(GetLangStr("25%",""),GetLangStr("25% alpha.","25%で透過する"),0xff-0x40);
      pmgi->CreateSet(GetLangStr("50%",""),GetLangStr("50% alpha.","50%で透過する"),0xff-0x80);
      pmgi->CreateSet(GetLangStr("75%",""),GetLangStr("75% alpha.","75%で透過する"),0xff-0xc0);
      pmgi->CreateSet(GetLangStr("100%",""),GetLangStr("Not draw BG.","背景を描画しない"),0xff-0xff);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Second bar and digial.","秒針とデジタル時計"),"",(u8*)&ps->SecDigi);
      pmgi->CreateSet(GetLangStr("None","無し"),GetLangStr("Clear","描画しない"),TProcState_SClock::ESD_None);
      pmgi->CreateSet(GetLangStr("Second","秒針"),GetLangStr("Second bar only","秒針のみ"),TProcState_SClock::ESD_SecOnly);
      pmgi->CreateSet(GetLangStr("Digital","デジタル時計"),GetLangStr("Digital clock only","デジタル時計のみ"),TProcState_SClock::ESD_DigiOnly);
      pmgi->CreateSet(GetLangStr("Both","両方"),GetLangStr("Draw both","両方描画する"),TProcState_SClock::ESD_Both);
      pmgi->SetDefault();
    }
    {
      CMenuGroupItem *pmgi=pmg->CreateItem(GetLangStr("Hour chars","時盤文字"),"",(u8*)&ps->HourChar);
      pmgi->CreateSet(GetLangStr("None","無し"),GetLangStr("Clear","描画しない"),TProcState_SClock::EHC_None);
      pmgi->CreateSet(GetLangStr("3/6/9/12",""),GetLangStr("3/6/9/12 hours only","3/6/9/12時のみ描画"),TProcState_SClock::EHC_ImpOnly);
      pmgi->CreateSet(GetLangStr("All","全て"),GetLangStr("Draw all","全て描画する"),TProcState_SClock::EHC_All);
      pmgi->SetDefault();
    }
    pmg->SetDefault();
  }
  
  pm->SetDefault();
  
//  pm->ShowTreeInfo();
}

