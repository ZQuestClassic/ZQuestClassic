unit zscript;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, LResources, Forms, Controls, Graphics, Dialogs,
  SynEdit, SynHighlighterCpp, SynHighlighterAny, Menus, SynHighlighterTeX,
  SynHighlighterMulti, SynMemo, SynUniHighlighter;

type

  { TForm2 }

  TForm2 = class(TForm)
    MainMenu1: TMainMenu;
    MenuItem1: TMenuItem;
    MenuItem10: TMenuItem;
    MenuItem11: TMenuItem;
    MenuItem12: TMenuItem;
    MenuItem13: TMenuItem;
    MenuItem14: TMenuItem;
    MenuItem15: TMenuItem;
    MenuItem17: TMenuItem;
    MenuItem18: TMenuItem;
    MenuItem19: TMenuItem;
    MenuItem20: TMenuItem;
    MenuItem21: TMenuItem;
    MenuItem22: TMenuItem;
    MenuItem7: TMenuItem;
    MenuItem8: TMenuItem;
    MenuItem9: TMenuItem;
    New1:      TMenuItem;
    MenuItem2: TMenuItem;
    MenuItem3: TMenuItem;
    MenuItem4: TMenuItem;
    MenuItem5: TMenuItem;
    MenuItem6: TMenuItem;
    openme:    TOpenDialog;
    PopupMenu1: TPopupMenu;
    saveme:    TSaveDialog;
    SynAnySyn1: TSynAnySyn;
    SynCppSyn1: TSynCppSyn;
    SynEdit1:  TSynEdit;
    highligtme: TSynUniSyn;
    SynMultiSyn1: TSynMultiSyn;
    procedure FormCreate(Sender: TObject);
    procedure MenuItem10Click(Sender: TObject);
    procedure MenuItem11Click(Sender: TObject);
    procedure MenuItem12Click(Sender: TObject);
    procedure MenuItem13Click(Sender: TObject);
    procedure MenuItem14Click(Sender: TObject);
    procedure MenuItem15Click(Sender: TObject);
    procedure MenuItem16Click(Sender: TObject);
    procedure MenuItem17Click(Sender: TObject);
    procedure MenuItem18Click(Sender: TObject);
    procedure MenuItem19Click(Sender: TObject);
    procedure MenuItem20Click(Sender: TObject);
    procedure MenuItem22Click(Sender: TObject);
    procedure MenuItem3Click(Sender: TObject);
    procedure MenuItem4Click(Sender: TObject);
    procedure MenuItem5Click(Sender: TObject);
    procedure MenuItem6Click(Sender: TObject);
    procedure MenuItem7Click(Sender: TObject);
    procedure MenuItem8Click(Sender: TObject);
    procedure MenuItem9Click(Sender: TObject);
    procedure New1Click(Sender: TObject);
    procedure SynEdit1Change(Sender: TObject);
  private
    { private declarations }
  public
    { public declarations }
  end;

var
  Form2: TForm2;
  zname: string;

implementation

{ TForm2 }

procedure TForm2.FormCreate(Sender: TObject);
begin

end;

procedure TForm2.MenuItem10Click(Sender: TObject);
begin
  synedit1.Undo;
end;

procedure TForm2.MenuItem11Click(Sender: TObject);
begin
  synedit1.Redo;
end;

procedure TForm2.MenuItem12Click(Sender: TObject);
begin
  synedit1.CopyToClipboard;
end;

procedure TForm2.MenuItem13Click(Sender: TObject);
begin
  synedit1.PasteFromClipboard;
end;

procedure TForm2.MenuItem14Click(Sender: TObject);
begin
  synedit1.undo;
end;

procedure TForm2.MenuItem15Click(Sender: TObject);
begin
  synedit1.CutToClipboard;
end;

procedure TForm2.MenuItem16Click(Sender: TObject);
begin
  synedit1.Redo;
end;

procedure TForm2.MenuItem17Click(Sender: TObject);
begin
  synedit1.CopyToClipboard;
end;

procedure TForm2.MenuItem18Click(Sender: TObject);
begin
  synedit1.CutToClipboard;
end;

procedure TForm2.MenuItem19Click(Sender: TObject);
begin
  synedit1.CopyToClipboard;
end;

procedure TForm2.MenuItem20Click(Sender: TObject);
begin
  synedit1.PasteFromClipboard;
end;

procedure TForm2.MenuItem22Click(Sender: TObject);
begin
  synedit1.SelectAll;
end;

procedure TForm2.MenuItem3Click(Sender: TObject);
begin
  openme.Execute;
  zname := openme.FileName;
  synedit1.Lines.Clear;
  synedit1.Lines.LoadFromFile(zname);
end;

procedure TForm2.MenuItem4Click(Sender: TObject);
begin
  try
    synedit1.Lines.SaveToFile(zname);
  except
    saveme.Execute;
    zname := saveme.FileName;
    synedit1.Lines.SaveToFile(zname);

  end;
end;

procedure TForm2.MenuItem5Click(Sender: TObject);
begin
  saveme.Execute;
  zname := saveme.FileName;
  synedit1.Lines.SaveToFile(zname);

end;

procedure TForm2.MenuItem6Click(Sender: TObject);
begin
  hide;
end;

procedure TForm2.MenuItem7Click(Sender: TObject);
begin
  synedit1.CopyToClipboard;
end;

procedure TForm2.MenuItem8Click(Sender: TObject);
begin
  synedit1.CutToClipboard;
end;

procedure TForm2.MenuItem9Click(Sender: TObject);
begin
  synedit1.SelectAll;
end;

procedure TForm2.New1Click(Sender: TObject);
begin
  synedit1.Lines.Clear;
end;

procedure TForm2.SynEdit1Change(Sender: TObject);
begin

end;

initialization
  {$I zscript.lrs}

end.
