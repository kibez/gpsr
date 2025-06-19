#ifndef obj_benefonH
#define obj_benefonH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_benefon_condition.h"
#include "obj_benefon_custom.h"
#include "car_auto_answer.h"

class car_benefon :
  public car_gsm,
  public icar_polling_packet,
  public icar_pdu_kind,
  public car_auto_answer,

  public icar_custom_ctrl,
  public cc_benefon_acknowlege,
  public cc_benefon_get_history,
  public cc_benefon_condition_activate,
  public cc_benefon_easy_track_minute,
  public cc_benefon_easy_track_second,
  public cc_benefon_easy_track_stop,
  public cc_benefon_complex_track,
//не реализовано
//  public cc_benefon_route_sending,
//  public cc_benefon_route_activate,
//  public cc_benefon_set_cfg,
//  public cc_benefon_get_cfg,

  public icar_condition_ctrl,
  public co_benefon_alert_attack,
  public co_benefon_login,
  public co_benefon_logout,
  public co_benefon_check_state_activate,
  public co_benefon_check_alarm,
  public co_benefon_triger_alarm,
  public co_benefon_triger_activate,
  public co_benefon_request_waypoint,
  public co_benefon_create_route,
  public co_benefon_state
{
public:
  enum {trgt_circular_area, //на пересечение зоны
        trgt_interval_message_limit, //переодически, остановка по к-ву сообщений
        trgt_interval_time, //переодически, остановка по истечении времени
        trgt_interval_date_time, //переодически, остановка при достижении временного порога
        trgt_speed,
        trgt_date_time,
        trgt_interval, //переодически, без ограничений
        trgt_real_time, //в режиме реального времени
        trgt_all_triggers=99 //вcе тригера
        };

  enum {trgc_continue,trgc_message_limit,trgc_duration_limit,trgc_datetime_limit};
  enum {cnf_always,cnf_always_code,cnf_no_confirmation};
  enum command_t{CND,TRI};
public:
  bool auto_answer_cnd;
  bool auto_answer_cnd_ok;
  bool auto_answer_trg;
  bool auto_answer_trg_ok;
public:
  car_benefon(int dev_id=OBJ_DEV_BENEFON,int obj_id=0) : car_gsm(dev_id,obj_id)
  {
    auto_answer_cnd=false;
    auto_answer_trg=false;
    auto_answer_cnd_ok=true;
    auto_answer_trg_ok=true;
    register_custom();register_condition();
  }

  void register_custom();
  void register_condition();

  car_benefon* clone(int obj_id=0){return new car_benefon(OBJ_DEV_BENEFON,obj_id);}

  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	bool params(void* param)
  {
    if(!car_gsm::params(param)) return false;
    auto_answer_cnd=false;
    auto_answer_trg=false;
    auto_answer_cnd_ok=true;
    auto_answer_trg_ok=true;
    if(pkr_get_one(param,"auto_answer_cnd",0,auto_answer_cnd)) return false;
    if(pkr_get_one(param,"auto_answer_trg",0,auto_answer_trg)) return false;
    if(pkr_get_one(param,"auto_answer_cnd_ok",0,auto_answer_cnd_ok)) return false;
    if(pkr_get_one(param,"auto_answer_trg_ok",0,auto_answer_trg_ok)) return false;
    return true;
  }

  const char* get_device_name() const;

  const static unsigned state_count=9; 

  static bool str2fix(const char* lat,const char* lon,fix_data& f,int &error,bool as_log=false);
  static bool time2fix(const char* data,fix_data& f,int &error);
  static bool date2fix(const char* data,fix_data& f,int &error);
  static bool speed2fix(const char* data,fix_data& f,int &error);
  static bool course2fix(const char* data,fix_data& f,int &error);

  static bool str_log2degree(const char* _data,double& val);
  static bool str2degree(const char* _data,double& val);

  static void degree2str(double latitude,double longitude,std::string& str_lat,std::string& str_lon);
  static std::string degree2str(double val);
};

#endif

