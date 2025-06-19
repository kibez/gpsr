#ifndef gpsH
#define gpsH
// Windows Header Files:
#include <windows.h>
#include<WINDOWSX.H>
#include<COMMCTRL.H>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <process.h>
#include <string>
#include <poll_error\poll_error_code.h>
#include <pokr/dbgprn.h>
#include "iinfdev.h"
#include <pkr_freezer.h>
#include <vector>

#include "resource.h"

#define TRAY_CALLBACK WM_USER + 101
#define CURRENT_IS_DEAD 12345
#define LOG_LOAD_TIMER_NEXT 12346

typedef unsigned ( __stdcall * START_ROUTINE)( void * );

///////////////////////////////////////////////
//
//  gps.c routine
//

bool ParseIniFile();
VOID PrintError(HWND hParent,PCHAR szCaption,DWORD Error);
void set_program_param(void* newpar);

///////////////////////////////////////////////
void AddMessage(std::string szMessage);
void CloseProgram();
void SetInformatorCfg(void* arg);
void save_ini();

//
// log param dialog
//
typedef struct _LOAD_LOG_PARAM{
	int year;
	int month;
	int day;
	int *ids;
	int ids_count;
	int ids_cur;
	char **name;
} LOAD_LOG_PARAM;

char MakeDialogLogParam(LOAD_LOG_PARAM* Param);

//
// log progress dialog
//
void set_log_progress(int cur,int max);
typedef int ( *log_progress_param)(void*);
int MakeDialogLogProgress(log_progress_param callback,void* arg);

//
//pokrishka export
//
int open_server();
void close_server();
bool is_server_opened();
void add_data(void* data,int can_wait);
void inc_data(void* data,int can_wait);
int inc_empties();
int conditions_empties();
void inc_condition_req(void* data);

int cfg_request();
void poll_error_inc(int obj_id,int close,int code,const char* info);
void poll_error_close(int obj_id);

extern bool pkr_auth_success;

//
// auth
//
bool ask_login_password(std::string& login,std::string& password);
extern std::vector<unsigned char> key;
void set_auth_key();

//
// 22.10.2002 direct request coordinat
//
void add_obj_req(void* data);
// 25.02.2003 custom request
void add_cust_req(void* data);
// 14.03.2003 condition request
void add_condition_req(void* data);

void add_old_param(void* &new_param,const void* old_param);

extern bool device_enabled;
void check_device_open();


struct local_t
{
  std::string path_to_key;
  bool ask_login_password;
  bool auto_update_cfg;

  bool log_program_message;//сообщения программы
  bool log_device_exchange;//трафик обмена между устройствами
  bool write_device_exchange_time;//писать время в лог обмена с устройством
  bool log_sms;//запись SMS
  bool log_ip;//запись IP пакетов
  bool log_synchro_packet;//запись синхро пакетов
  bool log_object;//запись сообщений объектам

  bool fix_gps_1024_weaks;	//фікс проблеми 1024 тижнів
  bool fix_gps_tezey_device;	//фікс девайса ел-механіки


  local_t();
  void* pack() const;
  bool unpack(void* d);
};

class device_cfg_t
{
  int dev_id;
public:
  const int& cdev_id;
  pkr_freezer data;

  device_cfg_t();
  device_cfg_t(const device_cfg_t& rhs);
  device_cfg_t& operator=(const device_cfg_t& rhs);
  void* pack() const;
  bool unpack(void* d);
  inline void clear(){data=0;}
};

class object_cfg_t
{
public:
  pkr_freezer data;

  object_cfg_t();
  object_cfg_t(const object_cfg_t& rhs);
  void* pack() const;
  bool unpack(void* d);
  inline void clear(){data=0;}
};

struct server_cfg_t
{
  std::string inet_host;
  int inet_port;
  bool is_tcp_cache;
  unsigned queue_len;
  int informator_id;
  std::string informator_name;

  server_cfg_t();
  void* pack() const;
  bool unpack(void* d);
  bool operator==(const server_cfg_t& rhs) const;
};

struct cfg_t
{
  local_t local;
  device_cfg_t device;
  server_cfg_t server;
  object_cfg_t objects;

  void* pack() const;
  bool unpack(void* d);
  inline void clear(){device.clear();objects.clear();}
};

extern cfg_t params;
void set_server_param(const server_cfg_t& val);

extern std::string program_directory;

#endif
