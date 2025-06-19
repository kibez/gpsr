#ifndef obj_wondex_customH
#define obj_wondex_customH
#include <pkr_freezer.h>
#include "../device_data/wondex/wondex_commands.h"
#include "../device_data/mobitel/mobitel_commands.h"
#include "../device_data/common_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Wondex
{
using namespace VisiPlug::Wondex;
using VisiPlug::enable_t;

class car_wondex;

template<int cust_id,class Req>
class cc_wondex_set_get : public icar_custom_packable<cust_id,Req,Req>
{
public:
  //параметры
  typedef typename Req req_t;
  //результат
  typedef typename Req res_t;
protected:
  std::string command_name;
  std::string in_prefix;
public:
  cc_wondex_set_get(const std::string& _command_name) : command_name(_command_name)
  {
    in_prefix="$OK:"+command_name+"=";
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
  {
    std::string str="$WP+"+command_name+"="+dynamic_cast<icar_password*>(this)->dev_password+",";

    if(req.set){if(!sbuild_custom_packet(req,str))return false;}
    else str+="?";
    str+="\r\n";

    ud.insert(ud.end(),str.begin(),str.end() );
    return true;
  }

  virtual bool sbuild_custom_packet(const req_t& req,std::string& str)=0l;

  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
  {
    if(car_wondex::parse_error(ud,command_name,res))return true;
    std::string str;
    str.append(ud.begin(),ud.end() );
    while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

    const char* in_get=strstr(str.c_str(),in_prefix.c_str());
    if(!in_get||str.c_str()!=in_get) return false;

    res.res_mask|=CUSTOM_RESULT_END;
    res_t v;
    v.set=false;
    if(sparse_custom_packet(in_get+in_prefix.size() ,v) )
    {
      res.res_mask|=CUSTOM_RESULT_DATA;
      pkr_freezer fr(v.pack());
      res.set_data(fr.get());
    }
    else res.err_code=POLL_ERROR_PARSE_ERROR;
    return true;
  }

  //Заглушка
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}

  virtual bool sparse_custom_packet(const char* str,res_t& val)=0l;
};



template<int cust_id,class Req=NullReq,class Res=NullReq>
class cc_wondex : public icar_custom_packable<cust_id,Req,Res>
{
public:
  //параметры
  typedef typename Req req_t;
  //результат
  typedef typename Res res_t;
protected:
  std::string command_name;
  std::string in_prefix;
public:
  cc_wondex(const std::string& _command_name) : command_name(_command_name)
  {
    in_prefix="$OK:"+command_name;
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
  {
    std::string str="$WP+"+command_name+"="+dynamic_cast<icar_password*>(this)->dev_password;

    if(!sbuild_custom_packet(req,str))return false;
    str+="\r\n";

    ud.insert(ud.end(),str.begin(),str.end() );
    return true;
  }

  virtual bool sbuild_custom_packet(const req_t& req,std::string& str)=0;

  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
  {
    if(car_wondex::parse_error(ud,command_name,res))return true;
    std::string str;
    str.append(ud.begin(),ud.end() );
    while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

    const char* in_get=strstr(str.c_str(),in_prefix.c_str());
    if(!in_get||str.c_str()!=in_get)
      return false;

    res.res_mask|=CUSTOM_RESULT_END;
    res_t v;
    if(sparse_custom_packet(in_get+in_prefix.size() ,v) )
      spack_custom(v,res,t2t<res_t>());
    else res.err_code=POLL_ERROR_PARSE_ERROR;
    return true;
  }

  //реализация интерфейса для команд с результатом
  template<class T>
  void spack_custom(const res_t& v,custom_result& res,t2t<T>)
  {
    pkr_freezer fr(v.pack());
    res.set_data(fr.get());
    res.res_mask|=CUSTOM_RESULT_DATA;
  }

  //реализация интерфейса для команд без результата
  void spack_custom(const res_t& v,custom_result& res,t2t<NullReq>)
  {
    res.res_mask|=CUSTOM_RESULT_DATA;
  }

  //Заглушка
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}

  virtual bool sparse_custom_packet(const char* str,res_t& val){return true;}
};


//
//-------------команды-------------------------------------------------------
//

struct cc_cfg : public cc_wondex_set_get<CURE_WONDEX_CFG,cfg_t>
{
  cc_cfg() : cc_wondex_set_get<CURE_WONDEX_CFG,cfg_t>("UNCFG"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_communication : public cc_wondex_set_get<CURE_WONDEX_COMMUNICATION,connect_t>
{
  cc_communication() : cc_wondex_set_get<CURE_WONDEX_COMMUNICATION,connect_t>("COMMTYPE"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_roaming : public cc_wondex_set_get<CURE_EASYTRAC_ROAM_LIST,enable_t>
{
  cc_roaming() : cc_wondex_set_get<CURE_EASYTRAC_ROAM_LIST,enable_t>("ROAMING"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_get_state : public icar_custom<CURE_INTELLITRAC_GET_STATE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_track : public cc_wondex_set_get<CURE_INTELLITRAC_TRACK,track_t>
{
  cc_track() : cc_wondex_set_get<CURE_INTELLITRAC_TRACK,track_t>("TRACK"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_log : public icar_custom<CURE_INTELLITRAC_LOG>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_receive_log : public virtual icar_custom<CURE_INTELLITRAC_RECEIVE_LOG>,
                                    public icar_custom_self_timeout<CURE_INTELLITRAC_RECEIVE_LOG>
{
  bool read_receive_log_count;
  int delay_start_time;
  int delay_stop_time;

  cc_receive_log(){read_receive_log_count=false;delay_start_time=0;delay_stop_time=0;packet_execute=1;}
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_reboot : public cc_wondex<CURE_INTELLITRAC_REBOOT>
{
  cc_reboot() : cc_wondex<CURE_INTELLITRAC_REBOOT>("REBOOT"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val){return true;}
};

struct cc_power : public cc_wondex_set_get<CURE_INTELLITRAC_POWER,pmgr_t>
{
  cc_power() : cc_wondex_set_get<CURE_INTELLITRAC_POWER,pmgr_t>("PSM"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_events_mask : public cc_wondex_set_get<CURE_EVENTS_MASK,event_mask_t>
{
  cc_events_mask() : cc_wondex_set_get<CURE_EVENTS_MASK,event_mask_t>("SETDR"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_reports : public cc_wondex_set_get<CURE_INTELLITRAC_REPORT,report_t>
{
  cc_reports() : cc_wondex_set_get<CURE_INTELLITRAC_REPORT,report_t>("SETEVT"){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_phones : public cc_wondex_set_get<CURE_INTELLITRAC_PHONE,phone_t>
{
  cc_phones() : cc_wondex_set_get<CURE_INTELLITRAC_PHONE,phone_t>("SETVIP"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_acc_power : public cc_wondex_set_get<CURE_WONDEX_ACC_POWER,acc_detect_t>
{
  cc_acc_power() : cc_wondex_set_get<CURE_WONDEX_ACC_POWER,acc_detect_t>("SACC"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_analog_input : public cc_wondex_set_get<CURE_INTELLITRAC_ANALOG_REPORT,analog_report_t>
{
  cc_analog_input() : cc_wondex_set_get<CURE_INTELLITRAC_ANALOG_REPORT,analog_report_t>("SETAE"){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_power_diff : public cc_wondex_set_get<CURE_WONDEX_POWER_DIFF,avl_t>
{
  cc_power_diff() : cc_wondex_set_get<CURE_WONDEX_POWER_DIFF,avl_t>("AVL"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_enable_events : public cc_wondex_set_get<CURE_WONDEX_ENABLE_EVENTS,integer_t>
{
  cc_enable_events() : cc_wondex_set_get<CURE_WONDEX_ENABLE_EVENTS,integer_t>("DISEV"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_clear_event_settings : public cc_wondex<CURE_WONDEX_CLEAR_EVENT,integer_t>
{
  cc_clear_event_settings() : cc_wondex<CURE_WONDEX_CLEAR_EVENT,integer_t>("CLEVT"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
};

struct cc_clear_report_queue : public cc_wondex<CURE_INTELLITRAC_CLEAR_REPORT_QUEUE>
{
  cc_clear_report_queue() : cc_wondex<CURE_INTELLITRAC_CLEAR_REPORT_QUEUE>("QBCLR"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val){return true;}
};

struct cc_imei : public cc_wondex<CURE_WONDEX_IMEI,NullReq,string_t>
{
  cc_imei() : cc_wondex<CURE_WONDEX_IMEI,NullReq,string_t>("IMEI"){in_prefix="$MSG:"+command_name+"=";}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_simid : public cc_wondex<CURE_WONDEX_SIMID,NullReq,string_t>
{
  cc_simid() : cc_wondex<CURE_WONDEX_SIMID,NullReq,string_t>("SIMID"){in_prefix="$MSG:"+command_name+"=";}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_gsm_info : public cc_wondex<CURE_WONDEX_GSM_INFO,NullReq,gsm_info_t>
{
  cc_gsm_info() : cc_wondex<CURE_WONDEX_GSM_INFO,NullReq,gsm_info_t>("GSMINFO"){in_prefix="$MSG:"+command_name+"=";}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_auto_report_bts : public cc_wondex_set_get<CURE_WONDEX_BTS,integer_t>
{
  cc_auto_report_bts() : cc_wondex_set_get<CURE_WONDEX_BTS,integer_t>("GBLAC"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_multi_report_bts : public cc_wondex_set_get<CURE_WONDEX_MBTS,track_t>
{
  cc_multi_report_bts() : cc_wondex_set_get<CURE_WONDEX_MBTS,track_t>("MGBLAC"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_baud : public cc_wondex_set_get<CURE_INTELLITRAC_BAUD,baud_t>
{
  cc_baud() : cc_wondex_set_get<CURE_INTELLITRAC_BAUD,baud_t>("SETBR"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_wiretap : public cc_wondex<CURE_INTELLITRAC_VOICE_WIRETAP,string_t>
{
  cc_wiretap() : cc_wondex<CURE_INTELLITRAC_VOICE_WIRETAP,string_t>("VWT"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
};

struct cc_version : public cc_wondex<CURE_INTELLITRAC_VERSION,NullReq,string_t>
{
  cc_version() : cc_wondex<CURE_INTELLITRAC_VERSION,NullReq,string_t>("VER"){in_prefix="$MSG:"+command_name+"=";}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_speed_report : public cc_wondex_set_get<CURE_INTELLITRAC_SPEED,speed_t>
{
  cc_speed_report() : cc_wondex_set_get<CURE_INTELLITRAC_SPEED,speed_t>("SPD"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_out : public cc_wondex<CURE_INTELLITRAC_OUTS,outs_t>
{
  cc_out() : cc_wondex<CURE_INTELLITRAC_OUTS,outs_t>("OUTC"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val){return true;}
};

struct cc_battery : public cc_wondex_set_get<CURE_INTELLITRAC_BATERY,enable_t>
{
  cc_battery() : cc_wondex_set_get<CURE_INTELLITRAC_BATERY,enable_t>("BATC"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_tow : public cc_wondex_set_get<CURE_INTELLIX1_TOW,tow_t>
{
  cc_tow() : cc_wondex_set_get<CURE_INTELLIX1_TOW,tow_t>("SETTOW"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_milege : public cc_wondex_set_get<CURE_INTELLITRAC_MILEAGE,mileage_t>
{
  cc_milege() : cc_wondex_set_get<CURE_INTELLITRAC_MILEAGE,mileage_t>("SETMILE"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_timer : public cc_wondex_set_get<CURE_INTELLITRAC_TIMER,timer_t>
{
  cc_timer() : cc_wondex_set_get<CURE_INTELLITRAC_TIMER,timer_t>("TMRR"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_mdt : public cc_wondex<CURE_INTELLITRAC_SEND_MDT,string_t>
{
  cc_mdt() : cc_wondex<CURE_INTELLITRAC_SEND_MDT,string_t>("CDMSG"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val){return true;}
};

struct cc_time_shift : public cc_wondex_set_get<CURE_TIME_SHIFT,integer_t>
{
  cc_time_shift() : cc_wondex_set_get<CURE_TIME_SHIFT,integer_t>("SETTZ"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_fkey : public cc_wondex_set_get<CURE_WONDEX_FKEY,fkey_t>
{
  cc_fkey() : cc_wondex_set_get<CURE_WONDEX_FKEY,fkey_t>("FKEY"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

}//namespace

#endif

