//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <memory>
//#include <iostream>

#include "Main.h"
#include "save_log_dialog.h"
#include "form_send_sms.h"
#include "sms_queue.h"
#include "iinfdev.h"
#include "gpsr_parameters.h"
#include "localsrv_pub.h"

//KIBEZ:BEGIN
/*
#include <boost/thread/condition.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/xtime.hpp>
*/
#include "boost_thread_stubs.h"
//KIBEZ:END

//---------------------------------------------------------------------------
//KIBEZ   #pragma link "trayicon"
#pragma resource "*.dfm"
TMainForm *MainForm;

HWND hWnd=0;

#define NUM_DIALOG_MESSAGE 50

UnicodeString informator_name;
//---------------------------------------------------------------------------
static bool need_auth_key=true;
static bool need_close=false;
static void* wait_cfg=0;

/* KIBEZ
typedef boost::recursive_mutex mtx_t;
boost::condition cnd;  // _any для работы с recursive_mutex
*/
typedef std::recursive_mutex mtx_t;
std::condition_variable_any cnd;  // _any для recursive_mutex

mtx_t mtx;

void AddMessage(std::string szMessage)
{
	dbg_print(szMessage.c_str());

  if(hWnd==0)return;
  UnicodeString* str=new UnicodeString(szMessage.c_str());
  PostMessage(hWnd,ADD_MESSAGE,(WPARAM)str,0);
}

void CloseProgram()
{
  if(hWnd)
  {
    PostMessage(hWnd,CLOSE_PROGRAM,0,0);
    return;
  }

  need_close=true;
  cnd.notify_one();
}

//bool LocalSrvFunc(const UnicodeString& cmd_line) // return true to exit program
bool LocalSrvFunc(const AnsiString& cmd_line) // return true to exit program
{
//KIBEZ  HMODULE lib = LoadLibrary(L"LocalSrv.dll");
HMODULE lib = LoadLibraryA("LocalSrv.dll");

  if(!lib) return false;
  //
  srv_listen listen = (srv_listen)GetProcAddress(lib, "listen");
  if(!listen) {FreeLibrary(lib); return false;}
  srv_send_command send_command = (srv_send_command)GetProcAddress(lib, "send_command");
  if(!send_command) {FreeLibrary(lib); return false;}
  srv_test test = (srv_test)GetProcAddress(lib, "test");
  if(!test) {FreeLibrary(lib); return false;}

  int pos = cmd_line.Pos(L"--start_as");
  if(pos)
  {
    UnicodeString srv_name(L"");
    pos += 10;
    while(pos < cmd_line.Length() && cmd_line[pos] == L' ') ++pos;

    while(pos < cmd_line.Length() && cmd_line[pos] != L' ')
    {
      srv_name += cmd_line[pos];
      ++pos;
    }
    if(srv_name.IsEmpty()) {FreeLibrary(lib); return true;}
    if(test(AnsiString(srv_name).c_str())) {FreeLibrary(lib); return true;}
    return !listen(AnsiString(srv_name).c_str(), CloseProgram);
  }

  pos = cmd_line.Pos(L"--stop");
  if(pos)
  {
    UnicodeString srv_name(L"");
    pos += 6;
    while(pos < cmd_line.Length() && cmd_line[pos] == L' ') ++pos;

    while(pos < cmd_line.Length() && cmd_line[pos] != L' ')
    {
      srv_name += cmd_line[pos];
      ++pos;
    }
    if(srv_name.IsEmpty()) {FreeLibrary(lib); return true;}
    UnicodeString stop_cmd = UnicodeString(L"stop ") + srv_name;
    send_command(AnsiString(srv_name).c_str(), AnsiString(stop_cmd).c_str());
    FreeLibrary(lib);
    return true;
  }
  FreeLibrary(lib);
  return false;
}

void SetInformatorCfg(void* arg)
{
	void* data=malloc(pkr_get_var_length(arg));
	memcpy(data,arg,pkr_get_var_length(arg));
  if(hWnd)
  {
    PostMessage(hWnd,INFORMATOR_HAVE_CFG,(WPARAM)data,0);
    return;
  }

//KIBEZ  mtx_t::scoped_lock lk(mtx);
  std::lock_guard<mtx_t> lk(mtx);

  if(wait_cfg)free(wait_cfg);
  wait_cfg=data;
  cnd.notify_one();
}

static void check_started()
{
	if(need_auth_key)
	{
		need_auth_key=false;
		set_auth_key();
	}

  check_device_open();

  open_server();
}


/* KIBEZ
void guiless_cycle()
{
  void* delay_cfg=0;

//KIBEZ  boost::xtime timeout;
  auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(60);

  while(!need_close)
  {
    if(delay_cfg)
    {
      set_program_param(delay_cfg);
      free(delay_cfg);
      delay_cfg=0;
    }

    check_started();

KIBEZ
//    mtx_t::scoped_lock lk(mtx);
//    xtime_get(&timeout, boost::TIME_UTC);
//    timeout.sec+=60;
//    if(!cnd.timed_wait(lk,timeout)) continue;


    std::unique_lock<mtx_t> lk(mtx);
ORG
//    auto timeout_point = std::chrono::steady_clock::now() + std::chrono::seconds(60);
//    if(cnd.wait_until(lk, timeout_point) == std::cv_status::timeout) continue;

    simple_time_point timeout_point = simple_chrono::now() + simple_chrono::seconds(60);
    if(cnd.wait_until(lk, 60000) == std::cv_status::timeout) continue; // 60000 ms = 60 sec


    delay_cfg=wait_cfg;
    wait_cfg=0;
  }
}
*/
void guiless_cycle()
{
  void* delay_cfg=0;

  while(!need_close)
  {
    if(delay_cfg)
    {
      set_program_param(delay_cfg);
      free(delay_cfg);
      delay_cfg=0;
    }

    check_started();

    std::unique_lock<mtx_t> lk(mtx);
    if(cnd.wait_until(lk, 60000) == std::cv_status::timeout) continue; // 60000 ms = 60 sec

    delay_cfg=wait_cfg;
    wait_cfg=0;
  }
}



__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
  orig_caption=Caption;
	Application->OnHint = ShowHint;
  hWnd=WindowHandle;
	add_message(L"Start...");
  inf_watch->Interval=150;
  inf_watch->Enabled=true;
  miDeviceOpen->Checked=device_enabled;
  informator_id_=0;

  try
  {
    tray->Visible=true;
  }
  catch(...){}
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::ShowHint(TObject *Sender)
{
	StatusLine->SimpleText = Application->Hint;
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FileExit(TObject *Sender)
{
	Close();
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::HelpContents(TObject *Sender)
{
	Application->HelpCommand(HELP_CONTENTS, 0);
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::HelpSearch(TObject *Sender)
{
	Application->HelpCommand(HELP_PARTIALKEY, reinterpret_cast<ULONG_PTR>(L""));
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::HelpHowToUse(TObject *Sender)
{
	Application->HelpCommand(HELP_HELPONHELP, 0);
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::HelpAbout(TObject *Sender)
{
	//---- Add code to show program's About Box ----
}
//----------------------------------------------------------------------------

void TMainForm::add_message(UnicodeString m)
{
  m=TDateTime::CurrentDateTime().FormatString(L"yyyy/mm/dd hh:nn:ss ")+m;
  mes->Lines->Add(m);

  while(mes->Lines->Count>NUM_DIALOG_MESSAGE)
    mes->Lines->Delete(0);

  int pos=0;
  for (int i=0;i<mes->Lines->Count-1;i++)
    pos+=mes->Lines->Strings[i].Length()+2;
  mes->SelStart=pos;
}

void __fastcall TMainForm::miSaveLogClick(TObject *Sender)
{
  std::unique_ptr<TSaveLogDialog> fm(new TSaveLogDialog(this));
  fm->execute();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::miGetCfgClick(TObject *Sender)
{
	if(cfg_request())
		Application->MessageBox(L"Помилка конфігурування",L"Віддалене конфігурування",MB_OK|MB_ICONERROR);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::inf_watchTimer(TObject *Sender)
{
  inf_watch->Enabled=false;
  check_started();

  inf_watch->Interval=60000;
  inf_watch->Enabled=true;
  if(informator_id_!=params.server.informator_id||informator_name_!=UnicodeString(params.server.informator_name.c_str()))
  {
    informator_id_=params.server.informator_id;
    UnicodeString id_str = UnicodeString(informator_id_);
    Caption=orig_caption+L" "+UnicodeString(params.server.informator_name.c_str())+L"("+id_str+L")";
    tray->Hint=Caption;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnInformatorCfg(TMessage& Message)
{
  set_program_param((void*)Message.WParam);
  free((void*)Message.WParam);
  inf_watch->Interval=150;
}

void __fastcall TMainForm::OnAddMessage(TMessage& Message)
{
  UnicodeString* str=(UnicodeString*)Message.WParam;
  add_message(*str);
  delete str;
}

void __fastcall TMainForm::miDeviceOpenClick(TObject *Sender)
{
  device_enabled=miDeviceOpen->Checked;
  check_device_open();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  inf_watch->Enabled=false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::miSaveLogFromFileClick(TObject *Sender)
{
  std::unique_ptr<TSaveLogDialog> fm(new TSaveLogDialog(this));
  fm->execute_archive();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::miOperationClick(TObject *Sender)
{
  miSaveLogFromFile->Enabled=pkr_auth_success;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::miSendSMSClick(TObject *Sender)
{
  SmsReqQueue* gm=dynamic_cast<SmsReqQueue*>(dev_entry);
	if(!gm)
	{
		AddMessage("SMS device not found");
		return;
	}

	if(!gm->sms_queue_ready())
	{
		AddMessage("SMS device not ready");
		return;
	}

	if(!formSendSMS->Execute()) return;
	gm=dynamic_cast<SmsReqQueue*>(dev_entry);
	if(!gm)
	{
		AddMessage("SMS device not found");
		return;
	}

  smsreq req;
  req.phone = AnsiString(formSendSMS->cbPhone->Text).c_str();
  AnsiString data_ansi = AnsiString(formSendSMS->mem->Text);
  std::string data_str = data_ansi.c_str();
  req.data.insert(req.data.end(), data_str.begin(), data_str.end());
  req.data=iso2gsm_alphabet(req.data);
  gm->add(req);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::miParametersClick(TObject *Sender)
{
  formParamters->cfg=params;
  if(!formParamters->show())return;

  const cfg_t& cfg=formParamters->cfg;

  bool f1=params.local.ask_login_password!=cfg.local.ask_login_password||
          params.local.path_to_key!=cfg.local.path_to_key;
  bool f2=!(params.server==cfg.server);

  params=cfg;

  if(params.local.log_program_message)dbg_level(7);
  else dbg_level(0);

  save_ini();
  if(f2)set_server_param(params.server);
  if(f1)
  {
    inf_watch->Enabled=false;
    inf_watch->Interval=120;
    need_auth_key=true;
    close_server();
    inf_watch->Enabled=true;

  }
  else if(f2) open_server();
}
//---------------------------------------------------------------------------
