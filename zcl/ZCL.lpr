program ZCL;

{$mode objfpc}{$H+}

uses
  Forms, Interfaces,
  zclmain {Form1},
  RunTimeTypeInfoControls;


//{$R *.res}

//{$R zcl.res}

{$IFDEF WINDOWS}{$R ZCL.rc}{$ENDIF}

begin
  Application.Title := 'ZC Launcher';
  Application.Initialize;
  Application.CreateForm(TForm1, Form1);
  form1.Show;
  Application.Run;
end.
