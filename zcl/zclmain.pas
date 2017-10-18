{Source Code for The Main zinterface of ZCL.
This source code is based off the ZCL 2.0 zinterface
being used to write for future Releases}



unit zclmain;

{$mode objfpc}{$H+}
interface

uses
  SysUtils, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls, LResources, ComCtrls, Buttons, Process,
  {$ifdef linux}
  unix,
  {$endif}
  inifiles, RTTICtrls, Classes;

type

  { TForm1 }

  TForm1 = class(TForm)
    Button2: TButton;
    Button3: TButton;
    multiinstance: TCheckBox;
    CheckBox2: TCheckBox;
    CheckBox34: TCheckBox;
    CheckBox35: TCheckBox;
    CheckBox36: TCheckBox;
    CheckBox37: TCheckBox;
    CheckBox38: TCheckBox;
    Image2: TImage;
    Image3: TImage;
    Image4: TImage;
    Image5: TImage;
    Image6: TImage;
    currentscheme: TImage;
    Label13: TLabel;
    Label19: TLabel;
    Label20: TLabel;
    Label21: TLabel;
    Memo1: TMemo;
    PageControl1: TPageControl;
    PageControl3: TPageControl;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    TabSheet3: TTabSheet;
    TabSheet4: TTabSheet;
    TabSheet7: TTabSheet;
    TabSheet8: TTabSheet;
    ttips: TCheckBox;
    combobrush: TCheckBox;
    floatbrush: TCheckBox;
    owriteprotect: TCheckBox;
    palcyc: TCheckBox;
    ruleset: TCheckBox;
    tprotect: TCheckBox;
    uncomp: TCheckBox;
    usestatic: TCheckBox;
    Label11: TLabel;
    Label12: TLabel;
    scaletext: TLabel;
    Label14: TLabel;
    Label15: TLabel;
    Label16: TLabel;
    Label17: TLabel;
    Label18: TLabel;
    scrmodezc: TComboBox;
    bcopy: TComboBox;
    bcminute: TComboBox;
    scopy: TComboBox;
    tehscheme: TComboBox;
    zqwinmode: TComboBox;
    scalemode: TComboBox;
    zqfpslimit: TComboBox;
    zinterface: TComboBox;
    zqsoundmode: TComboBox;
    zqshowfps: TComboBox;
    zclassic: TImage;
    zcqld: TComboBox;
    zcentrymode: TComboBox;
    zqsshot: TComboBox;
    zctitle: TComboBox;
    scrreszc: TComboBox;
    zclimfps: TComboBox;
    zcshowfps: TComboBox;
    zcskiplogo: TComboBox;
    zcqicons: TComboBox;
    zcbeep: TComboBox;
    zcsound: TComboBox;
    zcsshot: TComboBox;
    Image7: TImage;
    ImageList1: TImageList;
    Label1: TLabel;
    Label10: TLabel;
    Label2: TLabel;
    Label23: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    rez1: TComboBox;
    Image1: TImage;
    Memo4: TMemo;
    Notebook1: TPageControl;
    SpeedButton1: TSpeedButton;
    SpeedButton2: TSpeedButton;
    TIButton1: TTIButton;
    zelda20: TImage;
    tscreen: TImage;
    zeldanew: TImage;
    zquestopen: TOpenDialog;
    opennow: TOpenDialog;
    Page2: TPage;
    Page3: TPage;
    Page4: TPage;
    Page5: TPage;
    PageControl2: TPageControl;
    saveme: TSaveDialog;
    Process1: TProcess;
    Process2: TProcess;
    SCCheckbox8: TCheckbox;
    dx1: TCheckbox;
    ss1: TCheckbox;
    TabSheet5: TTabSheet;
    TabSheet6: TTabSheet;
    vsync1: TCheckbox;
    procedure bcminuteChange(Sender: TObject);
    procedure BitBtn1Click(Sender: TObject);
    procedure BitBtn2Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure floatbrushChange(Sender: TObject);
    procedure multiinstanceChange(Sender: TObject);
    procedure Page3BeforeShow(ASender: TObject; ANewPage: TPage;
      ANewIndex: integer);
    procedure rulesetChange(Sender: TObject);
    procedure palcycChange(Sender: TObject);
    procedure owriteprotectChange(Sender: TObject);
    procedure scopyChange(Sender: TObject);
    procedure tprotectChange(Sender: TObject);
    procedure ttipsChange(Sender: TObject);
    procedure combobrushChange(Sender: TObject);
    procedure scalemodeChange(Sender: TObject);
    procedure uncompChange(Sender: TObject);
    procedure usestaticChange(Sender: TObject);
    procedure tehschemeChange(Sender: TObject);
    procedure zqshowfpsChange(Sender: TObject);
    procedure zqfpslimitChange(Sender: TObject);
    procedure scrmodezcChange(Sender: TObject);
    procedure questmeClick(Sender: TObject);
    procedure CheckBox2Change(Sender: TObject);
    procedure CheckBox34Change(Sender: TObject);
    procedure CheckBox35Change(Sender: TObject);
    procedure CheckBox36Change(Sender: TObject);
    procedure CheckBox37Change(Sender: TObject);
    procedure CheckBox38Change(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure scrreszcChange(Sender: TObject);
    procedure zcbeepChange(Sender: TObject);
    procedure zcentrymodeChange(Sender: TObject);
    procedure zclimfpsChange(Sender: TObject);
    procedure zcqiconsChange(Sender: TObject);
    procedure zcqldChange(Sender: TObject);
    procedure zcshowfpsChange(Sender: TObject);
    procedure zcskiplogoChange(Sender: TObject);
    procedure zcsoundChange(Sender: TObject);
    procedure zcsshotChange(Sender: TObject);
    procedure zctitleChange(Sender: TObject);
    procedure zinterfaceChange(Sender: TObject);
    procedure zqsoundmodeChange(Sender: TObject);
    procedure bcopyChange(Sender: TObject);
    procedure zqsshotChange(Sender: TObject);
    procedure zqwinmodeChange(Sender: TObject);

  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;
  agcfg: Tinifile;

  //Open and Save
  fname: string;

  //New way it uses variables instead of TEdits XD
  ZCFullWindow: string;
  Res1: string;
  Sound1: string;
  Logo1: string;
  Quickload: string;
  zqsound: string;
  LoadQuest: string;
  ZQopen: string;
  ZCResX: string;
  ZCResY: string;
  ZCProcFix: string;
  ZQProcFix: string;
  MultiZC: string;

  //This is to launch ZC
  ZCLaunch: TProcess;

  //This is to Launch ZQ
  ZQLaunch: TProcess;

implementation


procedure TForm1.FormCreate(Sender: TObject);
begin
  //Settings Defaults This is for when you first start ZCL - ADJUST THIS SEAN! :-P
  {ZCFullWindow := '-windowed ';
  VsynchThrot   := '-v0 ';
  ZQLarge       := '-large ';
  MultiZC       := ' '; }

  //This part reads the settings if there is no settings then we will create the default options

  if not fileexists('ag.cfg') then
  begin
    memo1.Lines.SaveToFile('ag.cfg'); //Creating the new config with defaults.
  end;

  if fileexists('ag.cfg') then
  begin
    agcfg := Tinifile.Create('ag.cfg');
    ZCResX := agcfg.readstring('zeldadx', 'resx', '640');
    ZCResY := agcfg.readstring('zeldadx', 'resy', '480');

    //ZC Name Entry Mode
    if agcfg.readstring('zeldadx', 'name_entry_mode', '') = '0' then
    begin
      zcentrymode.Text := 'Keyboard';
    end;

    if agcfg.readstring('zeldadx', 'name_entry_mode', '') = '1' then
    begin
      zcentrymode.Text := 'Letter Grid';
    end;

    if agcfg.readstring('zeldadx', 'name_entry_mode', '') = '2' then
    begin
      zcentrymode.Text := 'Extended Letter Grid';
    end;

    //ZC Title Screen
    if agcfg.readstring('zeldadx', 'title', '') = '0' then
    begin
      zctitle.Text := 'Classic';
      tscreen.picture := zclassic.picture;
    end;

    if agcfg.readstring('zeldadx', 'title', '') = '1' then
    begin
      zctitle.Text := 'Modern';
      tscreen.picture := zelda20.picture;
    end;

    if agcfg.readstring('zeldadx', 'title', '') = '2' then
    begin
      zctitle.Text := 'New (2.5)';
      tscreen.picture := zeldanew.picture;
    end;

    //ZC Full Screen
    if agcfg.readstring('ZCL', 'zcfullscreen', '') = '1' then
    begin
      scrmodezc.Text := 'Full Screen';
      ZCFullWindow := '-fullscreen ';
    end
    else
    begin
      scrmodezc.Text := 'Window';
      ZCFullWindow := '-windowed ';
    end;

    //ZC Resolutions
    if (ZCResX = '320') and (ZCResY = '240') then
    begin
      scrreszc.Text := '320x240';
      Res1 := '-res 320 240 ';
    end
    else if (ZCResX = '640') and (ZCResY = '480') then
    begin
      scrreszc.Text := '640x480';
      Res1 := '-res 640 480 ';
    end
    else if (ZCResX = '960') and (ZCResY = '720') then
    begin
      scrreszc.Text := '960x720';
      Res1 := '-res 960 720 ';
    end
    else if (ZCResX = '1280') and (ZCResY = '960') then
    begin
      scrreszc.Text := '1280x960';
      Res1 := '-res 1280 960 ';
    end
    else if (ZCResX = '1600') and (ZCResY = '1200') then
    begin
      scrreszc.Text := '1600x1200';
      Res1 := '-res 1600 1200 ';
    end;

    //ZC Fps
    if agcfg.Readstring('zeldadx', 'throttlefps', '') = '1' then
    begin
      zclimfps.Text := 'Yes';
    end
    else
    begin
      zclimfps.Text := 'No';
    end;

    //ZC No sound
    if agcfg.readstring('ZCL', 'zcnosound', '') = '1' then
    begin
      zcsound.Text := 'Yes';
      Sound1 := '-s ';
    end
    else
    begin
      zcsound.Text := 'No';
      Sound1 := ' ';
    end;

    //ZC Skip Agn Logo
    if agcfg.readstring('ZCL', 'zcfast', '') = '1' then
    begin
      zcskiplogo.Text := 'Yes';
      Logo1 := '-fast ';
    end
    else
    begin
      zcskiplogo.Text := 'No';
      Logo1 := ' ';
    end;

    //ZC Quick Load
    if agcfg.readstring('ZCL', 'quickload', '') = '1' then
    begin
      zcqicons.Text := 'No';
      Quickload := '-quickload ';
      LoadQuest := '-load 1 ';
    end
    else
    begin
      zcqicons.Text := 'Yes';
      Quickload := ' ';
      LoadQuest := ' ';
    end;

    //ZC Multiple instances
    if agcfg.readstring('ZCL', 'multiple_instances', '') = '1' then
    begin
      MultiZC := '-multiple ';
      multiinstance.Checked := True;
    end
    else
    begin
      MultiZC := ' ';
    end;

    //ZQ Fullscreen / Window Mode
    if agcfg.readstring('zquest', 'fullscreen', '') = '1' then
    begin
      zqwinmode.Text := 'Full Screen';
    end
    else
    begin
      zqwinmode.Text := 'Window';

      if agcfg.ReadString('zquest', 'small', '') = '1' then
      begin
        scaletext.Visible := True;
        scalemode.Visible := True;
        if agcfg.ReadString('zquest', 'scale', '') = '1' then
        begin
          scalemode.Text := '1x';
        end;
        if agcfg.ReadString('zquest', 'scale', '') = '2' then
        begin
          scalemode.Text := '2x';
        end;
        if agcfg.ReadString('zquest', 'scale', '') = '3' then
        begin
          scalemode.Text := '3x';
        end;
        if agcfg.ReadString('zquest', 'scale', '') = '4' then
        begin
          scalemode.Text := '4x';
        end;
      end;
    end;

    //ZQ Large Mode
    if agcfg.ReadString('zquest', 'small', '') = '0' then
    begin
      zinterface.Text := 'New (2.5)';
    end
    else
    begin
      zinterface.Text := 'Classic';
    end;


    //ZQ No Sound
    if agcfg.ReadString('ZCL', 'zqnosound', '') = '1' then
    begin
      zqsoundmode.Text := 'Yes';
      ZQSound := '-s ';
    end
    else
    begin
      zqsoundmode.Text := 'No';
      zqsound := ' ';
    end;

    //ZC Show FPS :D
    if agcfg.ReadString('zeldadx', 'showfps', '') = '1' then
    begin
      zcshowfps.Text := 'Yes';
    end
    else
    begin
      zcshowfps.Text := 'No';
    end;

    //ZC Heart Beep
    if agcfg.ReadString('zeldadx', 'heart_beep', '') = '1' then
    begin
      zcbeep.Text := 'Yes';
    end
    else
    begin
      zcbeep.Text := 'No';
    end;

    //ZQ Limit FPS
    if agcfg.ReadString('zquest', 'vsync', '') = '1' then
    begin
      zqfpslimit.Text := 'Yes';
    end
    else
    begin
      zqfpslimit.Text := 'No';
    end;

    //ZQ Show FPS
    if agcfg.ReadString('zquest', 'showfps', '') = '1' then
    begin
      zqshowfps.Text := 'Yes';
    end
    else
    begin
      zqshowfps.Text := 'No';
    end;

    //ZQ Show Animations
    if agcfg.ReadString('zquest', 'animation_on', '') = '1' then
    begin
      checkbox35.Checked := True;
    end;

    //ZQ Open Last Quest
    if agcfg.ReadString('zquest', 'open_last_quest', '') = '1' then
    begin
      checkbox34.Checked := True;
    end;

    //ZQ Mouse Scroll
    if agcfg.ReadString('zquest', 'mouse_scroll', '') = '1' then
    begin
      checkbox36.Checked := True;
    end;

    //ZQSave Paths
    if agcfg.ReadString('zquest', 'save_paths', '') = '1' then
    begin
      checkbox37.Checked := True;
    end;

    //ZQ Combo Brush
    if agcfg.ReadString('zquest', 'combo_brush', '') = '1' then
    begin
      combobrush.Checked := True;
    end;

    //ZQ Tooltips
    if agcfg.ReadString('zquest', 'enable_tooltips', '') = '1' then
    begin
      ttips.Checked := True;
    end;

    //ZQ Float Brush
    if agcfg.ReadString('zquest', 'float_brush', '') = '1' then
    begin
      floatbrush.Checked := True;
    end;

    //ZQ Overwrite Protection
    if agcfg.ReadString('zquest', 'overwrite_prevention', '') = '1' then
    begin
      owriteprotect.Checked := True;
    end;

    //ZQ Cycle
    if agcfg.ReadString('zquest', 'cycle_on', '') = '1' then
    begin
      palcyc.Checked := True;
    end;

    //ZQ Ruleset
    if agcfg.ReadString('zquest', 'rulesetdialog', '') = '1' then
    begin
      ruleset.Checked := True;
    end;

    //ZQ Tile Protection
    if agcfg.ReadString('zquest', 'tile_protection', '') = '1' then
    begin
      tprotect.Checked := True;
    end;

    //ZQ Uncompressed Auto Saves
    if agcfg.ReadString('zquest', 'uncompressed_auto_saves', '') = '1' then
    begin
      uncomp.Checked := True;
    end;

    //ZQ Static for Invalid Data
    if agcfg.ReadString('zquest', 'invalid_static', '') = '1' then
    begin
      usestatic.Checked := True;
    end;

    //ZQShow Misaligns
    if agcfg.ReadString('zquest', 'show_misalignments', '') = '1' then
    begin
      checkbox38.Checked := True;
    end;

    //ZQ Screenshots
    if agcfg.ReadString('zquest', 'snapshot_format', '') = '0' then
    begin
      zqsshot.Text := 'BMP';
    end;
    if agcfg.ReadString('zquest', 'snapshot_format', '') = '1' then
    begin
      zqsshot.Text := 'GIF';
    end;
    if agcfg.ReadString('zquest', 'snapshot_format', '') = '2' then
    begin
      zqsshot.Text := 'Jpeg';
    end;
    if agcfg.ReadString('zquest', 'snapshot_format', '') = '3' then
    begin
      zqsshot.Text := 'PNG';
    end;
    if agcfg.ReadString('zquest', 'snapshot_format', '') = '4' then
    begin
      zqsshot.Text := 'PCX';
    end;
    if agcfg.ReadString('zquest', 'snapshot_format', '') = '5' then
    begin
      zqsshot.Text := 'TGA';
    end;


    //Zelda Snapshot
    if agcfg.ReadString('zeldadx', 'snapshot_format', '') = '0' then
    begin
      zcsshot.Text := 'BMP';
    end;
    if agcfg.ReadString('zeldadx', 'snapshot_format', '') = '1' then
    begin
      zcsshot.Text := 'GIF';
    end;
    if agcfg.ReadString('zeldadx', 'snapshot_format', '') = '2' then
    begin
      zcsshot.Text := 'Jpeg';
    end;
    if agcfg.ReadString('zeldadx', 'snapshot_format', '') = '3' then
    begin
      zcsshot.Text := 'PNG';
    end;
    if agcfg.ReadString('zeldadx', 'snapshot_format', '') = '4' then
    begin
      zcsshot.Text := 'PCX';
    end;
    if agcfg.ReadString('zeldadx', 'snapshot_format', '') = '5' then
    begin
      zcsshot.Text := 'TGA';
    end;

    //ZQ Auto Backup Copies
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '0' then
    begin
      bcopy.Text := '0';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '1' then
    begin
      bcopy.Text := '1';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '2' then
    begin
      bcopy.Text := '2';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '3' then
    begin
      bcopy.Text := '3';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '4' then
    begin
      bcopy.Text := '4';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '5' then
    begin
      bcopy.Text := '5';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '6' then
    begin
      bcopy.Text := '6';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '7' then
    begin
      bcopy.Text := '7';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '8' then
    begin
      bcopy.Text := '8';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '9' then
    begin
      bcopy.Text := '9';
    end;
    if agcfg.ReadString('zquest', 'auto_backup_retention', '') = '10' then
    begin
      bcopy.Text := '10';
    end;

    //ZQ Auto Save Interval
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '1' then
    begin
      scopy.Text := '1';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '2' then
    begin
      scopy.Text := '2';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '3' then
    begin
      scopy.Text := '3';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '4' then
    begin
      scopy.Text := '4';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '5' then
    begin
      scopy.Text := '5';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '6' then
    begin
      scopy.Text := '6';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '7' then
    begin
      scopy.Text := '7';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '8' then
    begin
      scopy.Text := '8';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '9' then
    begin
      scopy.Text := '9';
    end;
    if agcfg.readstring('zquest', 'auto_save_retention', '') = '10' then
    begin
      scopy.Text := '10';
    end;

    //ZQ Auto Save Copies
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '0' then
    begin
      bcminute.Text := 'Disabled';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '1' then
    begin
      bcminute.Text := '1 minute';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '2' then
    begin
      bcminute.Text := '2 minutes';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '3' then
    begin
      bcminute.Text := '3 minutes';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '4' then
    begin
      bcminute.Text := '4 minutes';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '5' then
    begin
      bcminute.Text := '5 minutes';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '6' then
    begin
      bcminute.Text := '6 minutes';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '7' then
    begin
      bcminute.Text := '7 minutes';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '8' then
    begin
      bcminute.Text := '8 minutes';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '9' then
    begin
      bcminute.Text := '9 minutes';
    end;
    if agcfg.readstring('zquest', 'auto_save_interval', '') = '10' then
    begin
      bcminute.Text := '10 minutes';
    end;

    //Color Scheme
    if agcfg.readString('zeldadx', 'gui_colorset', '') = '0' then
    begin
      tehscheme.Text := 'Default';
      currentscheme.Picture := image2.Picture;
    end;
    if agcfg.readString('zeldadx', 'gui_colorset', '') = '1' then
    begin
      tehscheme.Text := 'Windows 98';
      currentscheme.Picture := image3.Picture;
    end;
    if agcfg.readString('zeldadx', 'gui_colorset', '') = '687' then
    begin
      tehscheme.Text := 'Gold';
      currentscheme.Picture := image4.Picture;
    end;
    if agcfg.readString('zeldadx', 'gui_colorset', '') = '3' then
    begin
      tehscheme.Text := 'Blue';
      currentscheme.Picture := image5.Picture;
    end;
    if agcfg.readString('zeldadx', 'gui_colorset', '') = '4104' then
    begin
      tehscheme.Text := 'Pink';
      currentscheme.Picture := image6.Picture;
    end;
{$IFDEF WIN32}
    if agcfg.ReadString('graphics', 'disable_direct_updating', 'unset') = 'unset' then
    begin
      agcfg.WriteString('graphics', 'disable_direct_updating', '1');
    end;
    if agcfg.ReadString('zeldadx', 'use_dwm_flush', 'unset') = 'unset' then
    begin
      agcfg.WriteString('zeldadx', 'use_dwm_flush', '0');
    end;
    if agcfg.ReadString('zeldadx', 'zc_win_proc_fix', 'unset') = 'unset' then
    begin
      agcfg.WriteString('zeldadx', 'zc_win_proc_fix', '0');
    end;
    if agcfg.ReadString('zquest', 'zq_win_proc_fix', 'unset') = 'unset' then
    begin
      agcfg.WriteString('zquest', 'zq_win_proc_fix', '0');
    end;
{$ENDIF}
  end;

{$ifdef LINUX}
  if fileexists('zelda-l') then
  begin
  end
  else
  begin
    label23.Visible := True;
    speedbutton1.Enabled := False;
    speedbutton2.Enabled := False;
  end;
{$endif}

{$ifdef win32}
  if fileexists('zelda-w.exe') then
  begin
  end
  else
  begin
    label23.Visible := True;
    speedbutton1.Enabled := False;
    speedbutton2.Enabled := False;
  end;
{$endif}
end;

procedure TForm1.CheckBox2Change(Sender: TObject);
begin
  if checkbox2.Checked then
  begin
    zcqld.Enabled := True;
    zcqld.Text := 'Slot 1';
    LoadQuest := '-load 1';
  end
  else
  begin
    LoadQuest := ' ';
    zcqld.Enabled := False;
    zcqld.Text := '';
  end;
end;

procedure TForm1.CheckBox34Change(Sender: TObject);
begin
  if checkbox34.Checked then
  begin
    agcfg.WriteString('zquest', 'open_last_quest', '1');
  end
  else
  begin
    agcfg.WriteString('zquest', 'open_last_quest', '0');
  end;
end;

procedure TForm1.CheckBox35Change(Sender: TObject);
begin
  if checkbox35.Checked then
  begin
    agcfg.writestring('zquest', 'animation_on', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'animation_on', '0');
  end;
end;

procedure TForm1.CheckBox36Change(Sender: TObject);
begin
  if checkbox36.Checked then
  begin
    agcfg.writestring('zquest', 'mouse_scroll', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'mouse_scroll', '0');
  end;
end;

procedure TForm1.CheckBox37Change(Sender: TObject);
begin
  if checkbox37.Checked then
  begin
    agcfg.writestring('zquest', 'save_paths', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'save_paths', '0');
  end;

end;

procedure TForm1.CheckBox38Change(Sender: TObject);
begin
  if checkbox37.Checked then
  begin
    agcfg.writestring('zquest', 'show_misalignments', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'show_misalignments', '0');
  end;
end;

procedure TForm1.BitBtn1Click(Sender: TObject);
begin
{$IFDEF LINUX}
{
//process1.applicationname := 'zelda-l';
process1.CommandLine := './zelda-l ' + edit4.Caption + edit7.Caption + edit8.Caption + edit9.caption + edit10.caption;
process1.Active := true;
memo3.Lines.LoadFromFile('allegro.log');
}


  try
    unix.Shell('./zelda-l ' + MultiZC + ZCFullWindow + Res1 + Sound1 +
      Logo1 + Quickload + LoadQuest + ' & ')
  except
  end;
  //memo3.Lines.LoadFromFile('allegro.log');

{$ENDIF}

{$IFDEF WIN32}
  {This fixes the ZCL Freezing on launching ZC}
  process1.CommandLine := './zelda-w.exe ' + ZCFullWindow + Res1 +
    Sound1 + Logo1 + multizc + Quickload + LoadQuest;
  process1.Execute;

  //memo3.Lines.LoadFromFile('allegro.log');

{Old code
try
sysutils.WaitForThreadTerminate(false);
sysutils.ExecuteProcess('./zelda-w.exe ' , edit4.Caption + edit7.Caption + edit8.Caption + edit9.caption + edit10.caption )
except
end;
}
{$ENDIF}

end;

procedure TForm1.bcminuteChange(Sender: TObject);
begin
  if bcminute.Text = 'Disabled' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '0');
  end;
  if bcminute.Text = '1 minute' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '1');
  end;
  if bcminute.Text = '2 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '2');
  end;
  if bcminute.Text = '3 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '3');
  end;
  if bcminute.Text = '4 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '4');
  end;
  if bcminute.Text = '5 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '5');
  end;
  if bcminute.Text = '6 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '6');
  end;
  if bcminute.Text = '7 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '7');
  end;
  if bcminute.Text = '8 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '8');
  end;
  if bcminute.Text = '9 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '9');
  end;
  if bcminute.Text = '10 minutes' then
  begin
    agcfg.writestring('zquest', 'auto_save_interval', '10');
  end;
end;

procedure TForm1.BitBtn2Click(Sender: TObject);
begin
{$IFDEF LINUX}
  try
    unix.shell('./zquest-l ' + zqsound + ' & ');
  except
  end;

{$ENDIF}

{$IFDEF WIN32}
  process2.CommandLine := './zquest-w.exe' + zqsound;
  process2.Execute;

{$ENDIF}

end;

procedure TForm1.Button2Click(Sender: TObject);
begin
  try
    memo4.Lines.SaveToFile(fname);
  except
    saveme.Execute;
    fname := saveme.FileName;
    try
      memo4.Lines.SaveToFile(fname);
    except
      //This will do nothing
    end;

  end;
end;

procedure TForm1.Button3Click(Sender: TObject);
begin
  opennow.Execute;
  fname := opennow.FileName;
  memo4.Lines.Clear;
  try
    memo4.Lines.LoadFromFile(fname);
  except
    //Makes it not go omfg error
  end;

end;

procedure TForm1.floatbrushChange(Sender: TObject);
begin
  if floatbrush.Checked then
  begin
    agcfg.writestring('zquest', 'float_brush', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'float_brush', '0');
  end;
end;

procedure TForm1.multiinstanceChange(Sender: TObject);
begin
  if multiinstance.Checked then
  begin
    agcfg.writestring('ZCL', 'multiple_instances', '1');
    multizc := '-multiple ';
  end
  else
  begin
    agcfg.writestring('ZCL', 'multiple_instances', '0');
    multizc := '-multiple ';
  end;
end;

procedure TForm1.Page3BeforeShow(ASender: TObject; ANewPage: TPage; ANewIndex: integer);
begin

end;

procedure TForm1.rulesetChange(Sender: TObject);
begin
  if ruleset.Checked then
  begin
    agcfg.writestring('zquest', 'rulesetdialog', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'rulesetdialog', '0');
  end;
end;

procedure TForm1.palcycChange(Sender: TObject);
begin
  if palcyc.Checked then
  begin
    agcfg.writestring('zquest', 'cycle_on', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'cycle_on', '0');
  end;
end;

procedure TForm1.owriteprotectChange(Sender: TObject);
begin
  if owriteprotect.Checked then
  begin
    agcfg.writestring('zquest', 'overwrite_prevention', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'overwrite_prevention', '0');
  end;
end;

procedure TForm1.scopyChange(Sender: TObject);
begin
  if scopy.Text = '1' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '1');
  end;
  if scopy.Text = '2' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '2');
  end;
  if scopy.Text = '3' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '3');
  end;
  if scopy.Text = '4' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '4');
  end;
  if scopy.Text = '5' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '5');
  end;
  if scopy.Text = '6' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '6');
  end;
  if scopy.Text = '7' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '7');
  end;
  if scopy.Text = '8' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '8');
  end;
  if scopy.Text = '9' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '9');
  end;
  if scopy.Text = '10' then
  begin
    agcfg.writestring('zquest', 'auto_save_retention', '10');
  end;
end;

procedure TForm1.tprotectChange(Sender: TObject);
begin
  if tprotect.Checked then
  begin
    agcfg.writestring('zquest', 'tile_protection', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'tile_protection', '0');
  end;
end;

procedure TForm1.ttipsChange(Sender: TObject);
begin
  if ttips.Checked then
  begin
    agcfg.writestring('zquest', 'enable_tooltips', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'enable_tooltips', '0');
  end;
end;

procedure TForm1.combobrushChange(Sender: TObject);
begin
  if combobrush.Checked then
  begin
    agcfg.writestring('zquest', 'combo_brush', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'combo_brush', '0');
  end;
end;

procedure TForm1.scalemodeChange(Sender: TObject);
begin
  if scalemode.Text = '1x' then
  begin
    agcfg.writeString('zquest', 'scale', '1');
  end;
  if scalemode.Text = '2x' then
  begin
    agcfg.writeString('zquest', 'scale', '2');
  end;
  if scalemode.Text = '3x' then
  begin
    agcfg.writeString('zquest', 'scale', '3');
  end;
  if scalemode.Text = '4x' then
  begin
    agcfg.writeString('zquest', 'scale', '4');
  end;

end;

procedure TForm1.uncompChange(Sender: TObject);
begin
  if uncomp.Checked then
  begin
    agcfg.writestring('zquest', 'uncompressed_auto_saves', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'uncompressed_auto_saves', '0');
  end;
end;

procedure TForm1.usestaticChange(Sender: TObject);
begin
  if usestatic.Checked then
  begin
    agcfg.writestring('zquest', 'invalid_static', '1');
  end
  else
  begin
    agcfg.writestring('zquest', 'invalid_static', '0');
  end;
end;

procedure TForm1.tehschemeChange(Sender: TObject);
begin
  if tehscheme.Text = 'Default' then
  begin
    agcfg.WriteString('zeldadx', 'gui_colorset', '0');
    agcfg.WriteString('zquest', 'gui_colorset', '0');
    currentscheme.Picture := image2.Picture;
  end;
  if tehscheme.Text = 'Windows 98' then
  begin
    agcfg.WriteString('zeldadx', 'gui_colorset', '1');
    agcfg.WriteString('zquest', 'gui_colorset', '1');
    currentscheme.Picture := image3.Picture;
  end;
  if tehscheme.Text = 'Gold' then
  begin
    agcfg.WriteString('zeldadx', 'gui_colorset', '687');
    agcfg.WriteString('zquest', 'gui_colorset', '687');
    currentscheme.Picture := image4.Picture;
  end;
  if tehscheme.Text = 'Blue' then
  begin
    agcfg.WriteString('zeldadx', 'gui_colorset', '3');
    agcfg.WriteString('zquest', 'gui_colorset', '3');
    currentscheme.Picture := image5.Picture;
  end;
  if tehscheme.Text = 'Pink' then
  begin
    agcfg.WriteString('zeldadx', 'gui_colorset', '4104');
    agcfg.WriteString('zquest', 'gui_colorset', '4104');
    currentscheme.Picture := image6.Picture;
  end;
end;

procedure TForm1.zqshowfpsChange(Sender: TObject);
begin
  if zqshowfps.Text = 'Yes' then
  begin
    agcfg.writestring('zquest', 'showfps', '1');
  end;
  if zqshowfps.Text = 'No' then
  begin
    agcfg.writestring('zquest', 'showfps', '0');
  end;
end;

procedure TForm1.zqfpslimitChange(Sender: TObject);
begin
  if zqfpslimit.Text = 'Yes' then
  begin
    agcfg.writestring('zquest', 'vsync', '1');
  end;
  if zqfpslimit.Text = 'No' then
  begin
    agcfg.writestring('zquest', 'vsync', '0');
  end;
end;

procedure TForm1.scrmodezcChange(Sender: TObject);
begin
  if scrmodezc.Text = 'Full Screen' then
  begin
    ZCFullWindow := '-fullscreen ';
    agcfg.writestring('ZCL', 'zcfullscreen', '1');
  end;

  if scrmodezc.Text = 'Window' then
  begin
    ZCFullWindow := '-windowed ';
    agcfg.writestring('ZCL', 'zcfullscreen', '0');
  end;
end;

procedure TForm1.questmeClick(Sender: TObject);
begin
  zquestopen.Execute;
  agcfg.writestring('zquest', 'win_last_quest', zquestopen.FileName);

end;

procedure TForm1.zclimfpsChange(Sender: TObject);
begin
  if zclimfps.Text = 'Yes' then
  begin
    agcfg.writestring('zeldadx', 'throttlefps', '1');
  end;

  if zclimfps.Text = 'No' then
  begin
    agcfg.writestring('zeldadx', 'throttlefps', '0');
  end;
end;

procedure TForm1.zcqiconsChange(Sender: TObject);
begin
  if zcqicons.Text = 'No' then
  begin
    Quickload := '-quickload ';
    LoadQuest := '-load 1 ';
    agcfg.writestring('ZCL', 'quickload', '1');
  end;
  if zcqicons.Text = 'Yes' then
  begin
    Quickload := ' ';
    LoadQuest := ' ';
    agcfg.writestring('ZCL', 'quickload', '0');
  end;
end;

procedure TForm1.zcqldChange(Sender: TObject);
begin
  if zcqld.Text = 'Slot 1' then
  begin
    LoadQuest := '-load 1 ';
  end;

  if zcqld.Text = 'Slot 2' then
  begin
    LoadQuest := '-load 2 ';
  end;

  if zcqld.Text = 'Slot 3' then
  begin
    LoadQuest := '-load 3 ';
  end;

  if zcqld.Text = 'Slot 4' then
  begin
    LoadQuest := '-load 4 ';
  end;

  if zcqld.Text = 'Slot 5' then
  begin
    LoadQuest := '-load 5 ';
  end;

  if zcqld.Text = 'Slot 6' then
  begin
    LoadQuest := '-load 6 ';
  end;

  if zcqld.Text = 'Slot 7' then
  begin
    LoadQuest := '-load 7 ';
  end;

  if zcqld.Text = 'Slot 8' then
  begin
    LoadQuest := '-load 8 ';
  end;

  if zcqld.Text = 'Slot 9' then
  begin
    LoadQuest := '-load 9 ';
  end;

  if zcqld.Text = 'Slot 10' then
  begin
    LoadQuest := '-load 10 ';
  end;

  if zcqld.Text = 'Slot 11' then
  begin
    LoadQuest := '-load 11 ';
  end;

  if zcqld.Text = 'Slot 12' then
  begin
    LoadQuest := '-load 12 ';
  end;

  if zcqld.Text = 'Slot 13' then
  begin
    LoadQuest := '-load 13 ';
  end;

  if zcqld.Text = 'Slot 14' then
  begin
    LoadQuest := '-load 14 ';
  end;

  if zcqld.Text = 'Slot 15' then
  begin
    LoadQuest := '-load 15 ';
  end;
end;

procedure TForm1.zcshowfpsChange(Sender: TObject);
begin
  if zcshowfps.Text = 'Yes' then
  begin
    agcfg.writestring('zeldadx', 'showfps', '1');
  end;

  if zcshowfps.Text = 'No' then
  begin
    agcfg.writestring('zeldadx', 'showfps', '0');
  end;
end;

procedure TForm1.zcskiplogoChange(Sender: TObject);
begin
  if zcskiplogo.Text = 'Yes' then
  begin
    Logo1 := '-fast ';
    agcfg.writestring('ZCL', 'zcfast', '1');
  end;

  if zcskiplogo.Text = 'No' then
  begin
    Logo1 := ' ';
    agcfg.writestring('ZCL', 'zcfast', '0');
  end;
end;

procedure TForm1.zcsoundChange(Sender: TObject);
begin
  if zcsound.Text = 'Yes' then
  begin
    Sound1 := '-s ';
    agcfg.writestring('ZCL', 'zcnosound', '1');
  end;

  if zcsound.Text = 'No' then
  begin
    Sound1 := ' ';
    agcfg.writestring('ZCL', 'zcnosound', '0');
  end;
end;

procedure TForm1.zcsshotChange(Sender: TObject);
begin
  if zcsshot.Text = 'GIF' then
  begin
    agcfg.writestring('zeldadx', 'snapshot_format', '1');
  end;

  if zcsshot.Text = 'BMP' then
  begin
    agcfg.writestring('zeldadx', 'snapshot_format', '0');
  end;

  if zcsshot.Text = 'JPEG' then
  begin
    agcfg.writestring('zeldadx', 'snapshot_format', '2');
  end;
  if zcsshot.Text = 'PCX' then
  begin
    agcfg.writestring('zeldadx', 'snapshot_format', '4');
  end;
  if zcsshot.Text = 'TGA' then
  begin
    agcfg.writestring('zeldadx', 'snapshot_format', '5');
  end;
  if zcsshot.Text = 'PNG' then
  begin
    agcfg.writestring('zeldadx', 'snapshot_format', '3');
  end;
end;

procedure TForm1.zctitleChange(Sender: TObject);
begin
  if zctitle.Text = 'Classic' then
  begin
    tscreen.picture := zclassic.picture;
    agcfg.WriteString('zeldadx', 'title', '0');
  end;
  if zctitle.Text = 'Modern' then
  begin
    tscreen.picture := zelda20.picture;
    agcfg.WriteString('zeldadx', 'title', '1');
  end;
  if zctitle.Text = 'New (2.5)' then
  begin
    tscreen.picture := zeldanew.picture;
    agcfg.WriteString('zeldadx', 'title', '2');
  end;
end;

procedure TForm1.zinterfaceChange(Sender: TObject);
begin
  if zinterface.Text = 'Classic' then
  begin
    agcfg.writestring('zquest', 'small', '1');
    scaletext.Visible := True;
    scalemode.Visible := True;
    if agcfg.ReadString('zquest', 'scale', '') = '1' then
    begin
      scalemode.Text := '1x';
    end;
  end;
  if zinterface.Text = 'New (2.5)' then
  begin
    agcfg.writestring('zquest', 'small', '0');
    scaletext.Visible := False;
    scalemode.Visible := False;
  end;
end;

procedure TForm1.zqsoundmodeChange(Sender: TObject);
begin
  if zqsoundmode.Text = 'Yes' then
  begin
    ZQSound := '-s ';
    agcfg.writestring('ZCL', 'zqnosound', '1');
  end;

  if zqsoundmode.Text = 'No' then
  begin
    ZQSound := ' ';
    agcfg.writestring('ZCL', 'zqnosound', '0');
  end;

end;

procedure TForm1.bcopyChange(Sender: TObject);
begin
  if bcopy.Text = '0' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '0');
  end;
  if bcopy.Text = '1' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '1');
  end;
  if bcopy.Text = '2' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '2');
  end;
  if bcopy.Text = '3' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '3');
  end;
  if bcopy.Text = '4' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '4');
  end;
  if bcopy.Text = '5' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '5');
  end;
  if bcopy.Text = '6' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '6');
  end;
  if bcopy.Text = '7' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '7');
  end;
  if bcopy.Text = '8' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '8');
  end;
  if bcopy.Text = '9' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '9');
  end;
  if bcopy.Text = '10' then
  begin
    agcfg.writestring('zquest', 'auto_backup_retention', '10');
  end;

end;

procedure TForm1.zqsshotChange(Sender: TObject);
begin
  if zqsshot.Text = 'GIF' then
  begin
    agcfg.writestring('zquest', 'snapshot_format', '1');
  end;

  if zqsshot.Text = 'BMP' then
  begin
    agcfg.writestring('zquest', 'snapshot_format', '0');
  end;

  if zqsshot.Text = 'JPEG' then
  begin
    agcfg.writestring('zquest', 'snapshot_format', '2');
  end;
  if zqsshot.Text = 'PCX' then
  begin
    agcfg.writestring('zquest', 'snapshot_format', '4');
  end;
  if zqsshot.Text = 'TGA' then
  begin
    agcfg.writestring('zquest', 'snapshot_format', '5');
  end;
  if zqsshot.Text = 'PNG' then
  begin
    agcfg.writestring('zquest', 'snapshot_format', '3');
  end;
end;

procedure TForm1.zqwinmodeChange(Sender: TObject);
begin
  if zqwinmode.Text = 'Full Screen' then
  begin
    agcfg.writestring('zquest', 'fullscreen', '1');
    agcfg.writestring('zquest', 'scale', '1');
    scaletext.Visible := False;
    scalemode.Visible := False;
  end;

  if zqwinmode.Text = 'Window' then
  begin
    agcfg.writestring('zquest', 'fullscreen', '0');
    scaletext.Visible := True;
    scalemode.Visible := True;

    if agcfg.readstring('zquest', 'small', '') = '1' then
    begin
      scaletext.Visible := True;
      scalemode.Visible := True;
      scalemode.Text := '1x';

      if agcfg.ReadString('zquest', 'scale', '') = '1' then
      begin
        scalemode.Text := '1x';
      end;
      if agcfg.ReadString('zquest', 'scale', '') = '2' then
      begin
        scalemode.Text := '2x';
      end;
      if agcfg.ReadString('zquest', 'scale', '') = '3' then
      begin
        scalemode.Text := '3x';
      end;
      if agcfg.ReadString('zquest', 'scale', '') = '4' then
      begin
        scalemode.Text := '4x';
      end;
    end
    else
    begin
      scaletext.Visible := False;
      scalemode.Visible := False;
    end;
  end;
end;

procedure TForm1.scrreszcChange(Sender: TObject);
begin
  if scrreszc.Text = '320x240' then
  begin
    Res1 := '-res 320 240 ';
    agcfg.writestring('zeldadx', 'resx', '320');
    agcfg.writestring('zeldadx', 'resy', '240');
  end
  else if scrreszc.Text = '640x480' then
  begin
    Res1 := '-res 640 480 ';
    agcfg.writestring('zeldadx', 'resx', '640');
    agcfg.writestring('zeldadx', 'resy', '480');
  end
  else if scrreszc.Text = '960x720' then
  begin
    Res1 := '-res 960 720 ';
    agcfg.writestring('zeldadx', 'resx', '960');
    agcfg.writestring('zeldadx', 'resy', '720');
  end
  else if scrreszc.Text = '1280x960' then
  begin
    Res1 := '-res 1280 960 ';
    agcfg.writestring('zeldadx', 'resx', '1280');
    agcfg.writestring('zeldadx', 'resy', '960');
  end
  else if scrreszc.Text = '1600x1200' then
  begin
    Res1 := '-res 1600 1200 ';
    agcfg.writestring('zeldadx', 'resx', '1600');
    agcfg.writestring('zeldadx', 'resy', '1200');
  end;
end;

procedure TForm1.zcbeepChange(Sender: TObject);
begin
  if zcbeep.Text = 'Yes' then
  begin
    agcfg.writestring('zeldadx', 'heart_beep', '1');
  end;
  if zcbeep.Text = 'No' then
  begin
    agcfg.writestring('zeldadx', 'heart_beep', '0');
  end;
end;

procedure TForm1.zcentrymodeChange(Sender: TObject);
begin
  if zcentrymode.Text = 'Keyboard' then
  begin
    agcfg.writestring('zeldadx', 'name_entry_mode', '0');
  end;

  if zcentrymode.Text = 'Letter Grid' then
  begin
    agcfg.writestring('zeldadx', 'name_entry_mode', '1');
  end;

  if zcentrymode.Text = 'Extended Letter Grid' then
  begin
    agcfg.writestring('zeldadx', 'name_entry_mode', '2');
  end;
end;

initialization
  {$i zclmain.lrs}


end.


