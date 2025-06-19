//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
#include <Winapi.Windows.hpp>
#include <Winapi.Messages.hpp>
#include <System.SysUtils.hpp>
#include <System.Classes.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <System.Classes.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include "trayicon.h"
#include <Vcl.ImgList.hpp>
#include <Xml.xmldom.hpp>
#include <Xml.XMLDoc.hpp>
#include <Xml.XMLIntf.hpp>
#include <System.ImageList.hpp>
//---------------------------------------------------------------------------
#include <string>
#include <pokr/dbgprn.h>
#include "gps.h"
#include <libpokr/libpokrexp.h>

#define INFORMATOR_HAVE_CFG WM_USER+1
#define CLOSE_PROGRAM WM_USER+2
#define ADD_MESSAGE WM_USER+3

class TMainForm : public TForm
{
__published:   
	TMainMenu *MainMenu;
  TMenuItem *miFileExitItem;
  TMenuItem *miHelpContentsItem;
  TMenuItem *miHelpSearchItem;
  TMenuItem *miHelpHowToUseItem;
  TMenuItem *miHelpAboutItem;
	TStatusBar *StatusLine;
  TMemo *mes;
  TTimer *inf_watch;
  TMenuItem *miBreak;
  TMenuItem *miSaveLog;
  TMenuItem *miGetCfg;
  TMenuItem *miParameters;
  TTrayIcon *tray;
  TImageList *img;
  TMenuItem *miBreak1;
  TMenuItem *miOperation;
  TMenuItem *miHelpMenu;
  TMenuItem *miDeviceOpen;
  TMenuItem *miSaveLogFromFile;
  TMenuItem *miSendSMS;
	void __fastcall ShowHint(TObject *Sender);
	void __fastcall FileExit(TObject *Sender);
	void __fastcall HelpContents(TObject *Sender);
	void __fastcall HelpSearch(TObject *Sender);
	void __fastcall HelpHowToUse(TObject *Sender);
	void __fastcall HelpAbout(TObject *Sender);
  void __fastcall miSaveLogClick(TObject *Sender);
  void __fastcall miGetCfgClick(TObject *Sender);
  void __fastcall inf_watchTimer(TObject *Sender);
  void __fastcall miDeviceOpenClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall miSaveLogFromFileClick(TObject *Sender);
  void __fastcall miOperationClick(TObject *Sender);
  void __fastcall miSendSMSClick(TObject *Sender);
  void __fastcall miParametersClick(TObject *Sender);
private:        // private user declarations
  void __fastcall OnInformatorCfg(TMessage& Message);
  void __fastcall OnCloseProgram(TMessage& Message){Close();}
  void __fastcall OnAddMessage(TMessage& Message);

  UnicodeString orig_caption;
  int informator_id_;
  UnicodeString informator_name_;
public:         // public user declarations
	virtual __fastcall TMainForm(TComponent* Owner);
  void add_message(UnicodeString m);

 BEGIN_MESSAGE_MAP
        MESSAGE_HANDLER(INFORMATOR_HAVE_CFG,TMessage,OnInformatorCfg)
        MESSAGE_HANDLER(CLOSE_PROGRAM,TMessage,OnCloseProgram)
        MESSAGE_HANDLER(ADD_MESSAGE,TMessage,OnAddMessage)
 END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif