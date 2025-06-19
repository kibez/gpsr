//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <libpokr/libpokrexp.h>
#include "gps.h"
#include "iinfdev.h"
//
#include <windows.h>

#pragma hdrstop

//---------------------------------------------------------------------------
USEFORM("logpass_dialog.cpp", LogpassDialog);
USEFORM("save_log_dialog.cpp", SaveLogDialog);
USEFORM("form_send_sms.cpp", formSendSMS);
USEFORM("form_gps_log_date.cpp", formGpsLogDate);
USEFORM("gpsr_parameters.cpp", formParamters);
USEFORM("Main.cpp", MainForm);
//---------------------------------------------------------------------------
HINSTANCE hInst;
std::vector<unsigned char> key;
void guiless_cycle();

bool LocalSrvFunc(const AnsiString& cmd_line); // return true to exit program

SERVICE_STATUS gSvcStatus;
SERVICE_STATUS_HANDLE gSvcStatusHandle;

void SvcManage(bool install);
void WINAPI SvcCtrlHandler( DWORD dwCtrl);
void WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv);
void ReportSvcStatus(DWORD dwCurrentState,DWORD dwWin32ExitCode,DWORD dwWaitHint);
void SvcReportEvent(LPTSTR szFunction);
AnsiString GetServiceName();

void imain(const AnsiString& cmd_line);

WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR _cmd_line, int)
{
/*
	- SEM_NOGPFAULTERRORBOX

	Отключает стандартный диалог Windows "Приложение выполнило недопустимую операцию и будет закрыто"
	Вместо диалога - приложение просто молча завершается или вызывается обработчик исключений
	Полезно для серверных приложений, где не должно быть UI

	- SEM_FAILCRITICALERRORS

	Отключает диалоги критических системных ошибок
	Например: "Диск не готов", "Файл не найден на съемном носителе" и т.д.
	Вместо диалога - функции возвращают код ошибки
*/

  SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);

  //
  AnsiString cmd_line(_cmd_line);
  if(LocalSrvFunc(cmd_line)) return 0;
  if(cmd_line.Pos("--stop")!=0) return 0;

  if(cmd_line.Pos("--register")!=0)
  {
    SvcManage(true);
    return 0;
  }

  if(cmd_line.Pos("--unregister")!=0)
  {
    SvcManage(false);
    return 0;
  }

  if(cmd_line.Pos("--service")!=0)
  {
    AnsiString svc_name=GetServiceName();
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { svc_name.c_str(),(LPSERVICE_MAIN_FUNCTION) SvcMain },
        { NULL, NULL }
    };

    if(!StartServiceCtrlDispatcher(DispatchTable))
        SvcReportEvent("StartServiceCtrlDispatcher() failed");
    return 0;
  }

  imain(cmd_line);
  return 0;
}

bool pre_start()
{
  AnsiString prg_dir=ExcludeTrailingPathDelimiter(ExtractFilePath(Application->ExeName))+"\\";
  SetCurrentDir(prg_dir);

  AnsiString dbg_file=prg_dir+"gpsr.log";
  dbg_fopen(dbg_file.c_str());
  dbg_time_milliseconds(1);
  infs.initialize();

  if(!ParseIniFile())
  {
    dbg_print("Coudn't parse cfg");
    return false;
  }

  if(params.local.log_program_message)dbg_level(7);
  else dbg_level(0);

  infs.set_active(params.device.cdev_id);

  set_server_param(params.server);

  dev_entry->device_params(params.device.data.get());
  dev_entry->objects_params(params.objects.data.get());
  check_device_open();
  dev_entry->start();

  program_directory=prg_dir.c_str();

  return true;
}

void pre_stop()
{
  dbg_print("pre_stop()1");
  dev_entry->stop();
  dbg_print("pre_stop()2");
  close_server();
  dbg_print("pre_stop()3");

  infs.destruct();

  dbg_print("pre_stop()4");
  dbg_fclose();

  params.clear();
}

void imain(const AnsiString& cmd_line)
{
  if(!pre_start())return;
  bool silent=cmd_line.Pos("--silent")!=0;

  try
  {
     Application->Initialize();
     if(silent)guiless_cycle();
     else
     {
       Application->CreateForm(__classid(TMainForm), &MainForm);
     Application->CreateForm(__classid(TLogpassDialog), &LogpassDialog);
     Application->CreateForm(__classid(TformSendSMS), &formSendSMS);
     Application->CreateForm(__classid(TformGpsLogDate), &formGpsLogDate);
     Application->CreateForm(__classid(TformParamters), &formParamters);
     Application->Run();
     }
  }
  catch (Exception &exception)
  {
     Application->ShowException(&exception);
  }
  catch (...)
  {
     try
     {
       throw Exception("unknown exception");
     }
     catch (Exception &exception)
     {
       Application->ShowException(&exception);
     }
  }

  pre_stop();
}

AnsiString GetServiceName()
{
    AnsiString svc_name=Application->ExeName;
    svc_name=StringReplace(svc_name,"\\","_",TReplaceFlags()<<rfReplaceAll);
    svc_name=StringReplace(svc_name,"/","_",TReplaceFlags()<<rfReplaceAll);
    svc_name=StringReplace(svc_name,":","_",TReplaceFlags()<<rfReplaceAll);
    return svc_name;
}

void SvcManage(bool install)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(!schSCManager)
    {
//      Application->MessageBox("OpenSCManager() failed",Application->ExeName.c_str());
      Application->MessageBox(L"OpenSCManager() failed", Application->ExeName.c_str());
      return;
    }

    AnsiString svc_name=GetServiceName();
    AnsiString cmd_line="\""+Application->ExeName+"\""+" --service";

    if(install)
    {
      schService=CreateService(
          schSCManager,              // SCM database
          svc_name.c_str(),                   // name of service
          svc_name.c_str(),                   // service name to display
          SERVICE_ALL_ACCESS,        // desired access
          SERVICE_WIN32_OWN_PROCESS, // service type
          SERVICE_AUTO_START,      // start type
          SERVICE_ERROR_NORMAL,      // error control type
          cmd_line.c_str(),                    // path to service's binary
          NULL,                      // no load ordering group
          NULL,                      // no tag identifier
          NULL,                      // no dependencies
          NULL,                      // LocalSystem account
          NULL);                     // no password

      if(!schService)
	Application->MessageBox(L"CreateService() failed", UnicodeString(svc_name).c_str());
    }
    else
    {
      schService=OpenService(schSCManager,svc_name.c_str(),SERVICE_ALL_ACCESS);
      if(!schService)Application->MessageBox(L"OpenService() failed", UnicodeString(svc_name).c_str() );

      if(schService)
      {
        if(!DeleteService(schService))
          Application->MessageBox(L"DeleteService() failed", UnicodeString(svc_name).c_str() );
      }
    }

    if(schService)CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
  AnsiString svc_name=GetServiceName();

  gSvcStatusHandle=RegisterServiceCtrlHandler(svc_name.c_str(),SvcCtrlHandler);
  if(!gSvcStatusHandle)
  {
    SvcReportEvent("RegisterServiceCtrlHandler() failed");
    return;
  }

  gSvcStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
  gSvcStatus.dwServiceSpecificExitCode=0;
  ReportSvcStatus(SERVICE_RUNNING,NO_ERROR,0);

  if(!pre_start())
  {
    ReportSvcStatus(SERVICE_STOPPED,NO_ERROR,0);
    return;
  }

  try
  {
     Application->Initialize();
     guiless_cycle();
  }
  catch (Exception &exception)
  {
     Application->ShowException(&exception);
  }
  catch (...)
  {
     try
     {
       throw Exception("unknown exception");
     }
     catch (Exception &exception)
     {
       Application->ShowException(&exception);
     }
  }

  ReportSvcStatus( SERVICE_STOP_PENDING, NO_ERROR, 0 );
  pre_stop();
  ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
}

void ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint=1;

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)gSvcStatus.dwControlsAccepted=0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED) ) gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

void WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
  if(dwCtrl==SERVICE_CONTROL_STOP)
  {
    ReportSvcStatus(SERVICE_STOP_PENDING,NO_ERROR,0);
    CloseProgram();
  }
}

void SvcReportEvent(LPTSTR szFunction)
{
    AnsiString svc_name=GetServiceName();
    HANDLE hEventSource=RegisterEventSource(NULL, svc_name.c_str());
    if(hEventSource==0)return;

    LPCTSTR lpszStrings[2];
    lpszStrings[0] = svc_name.c_str();
    lpszStrings[1] = szFunction;

    ReportEvent(hEventSource,EVENTLOG_ERROR_TYPE,0,0,NULL,2,0,lpszStrings,NULL);
    DeregisterEventSource(hEventSource);
}

//---------------------------------------------------------------------------