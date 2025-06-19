#ifndef obj_intellitrack_customH
#define obj_intellitrack_customH
#include <pkr_freezer.h>
#include "../device_data/intellitrac/intellitrac_commands.h"
#include "../device_data/mobitel/mobitel_commands.h"
#include "../device_data/common_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Intellitrac
{
using namespace VisiPlug::Intellitrac;
using VisiPlug::Mobitel::password;
using VisiPlug::Mobitel::phone;
using VisiPlug::string_t;
using VisiPlug::universal_command_t;
using VisiPlug::enable_t;

template<int cust_id,class Req>
class cc_intellitrac_set_get : public icar_custom_packable<cust_id,Req,Req>
{
public:
  //параметры
  typedef typename Req req_t;
  //результат
  typedef typename Req res_t;
private:
  static const char* command_name;

  std::string in_set_prefix;
  std::string in_get_prefix;
public:
  void init(icar_custom_ctrl* d)
  {
    icar_custom<cust_id>::init(d);
    in_set_prefix=std::string("OK:")+command_name;
    in_get_prefix=dynamic_cast<car_intellitrack*>(d)->build_in_get_prefix(command_name);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
  {
    std::string str=std::string("$ST+")+command_name+"="+dynamic_cast<icar_password*>(this)->dev_password+",";

    if(req.set){if(!sbuild_custom_packet(req,str))return false;}
    else str+="?";
    str+="\r\n";

    ud.insert(ud.end(),str.begin(),str.end() );
    return true;
  }

  virtual bool sbuild_custom_packet(const req_t& req,std::string& str)=0l;

  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
  {
    if(car_intellitrack::parse_error(ud,res))return true;
    std::string str;
    str.append(ud.begin(),ud.end() );
    while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

    const char* in_get=strstr(str.c_str(),in_get_prefix.c_str());

    if(in_get&&str.c_str()+1==in_get)
    {
      res.res_mask|=CUSTOM_RESULT_END;
      res_t v;
      v.set=false;
      if(sparse_custom_packet(in_get+in_get_prefix.size() ,v) )
      {
        res.res_mask|=CUSTOM_RESULT_DATA;
        pkr_freezer fr(v.pack());
        res.set_data(fr.get());
      }
      else res.err_code=POLL_ERROR_PARSE_ERROR;
      return true;
    }
    else if(str.find(in_set_prefix)!=std::string::npos)
    {
      res.res_mask|=CUSTOM_RESULT_END;
      return true;
    }

    return false;
  }

  //Заглушка
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}

  virtual bool sparse_custom_packet(const char* str,res_t& val)=0l;
};



template<int cust_id,class Req=NullReq,class Res=NullReq>
class cc_intellitrac : public icar_custom_packable<cust_id,Req,Res>
{
public:
  //параметры
  typedef typename Req req_t;
  //результат
  typedef typename Res res_t;
private:
  static const char* command_name;

  std::string in_set_prefix;
  std::string in_get_prefix;
public:
  void init(icar_custom_ctrl* d)
  {
    icar_custom<cust_id>::init(d);
    in_set_prefix=std::string("OK:")+command_name;
    in_get_prefix=dynamic_cast<car_intellitrack*>(d)->build_in_get_prefix(command_name);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
  {
    std::string str=std::string("$ST+")+command_name+"="+dynamic_cast<icar_password*>(this)->dev_password;

    if(!sbuild_custom_packet(req,str))return false;
    str+="\r\n";

    ud.insert(ud.end(),str.begin(),str.end() );
    return true;
  }

  virtual bool sbuild_custom_packet(const req_t& req,std::string& str)=0;

  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
  {
    if(car_intellitrack::parse_error(ud,res))return true;
    std::string str;
    str.append(ud.begin(),ud.end() );
    while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

    const char* in_get=strstr(str.c_str(),in_get_prefix.c_str());

    if(in_get&&str.c_str()+1==in_get)
    {
      res.res_mask|=CUSTOM_RESULT_END;
      res_t v;
      if(sparse_custom_packet(in_get+in_get_prefix.size() ,v) )
        spack_custom(v,res,t2t<res_t>());
      else res.err_code=POLL_ERROR_PARSE_ERROR;
      return true;
    }
    else if(str.find(in_set_prefix)!=std::string::npos)
    {
      res.res_mask|=CUSTOM_RESULT_END;
      return true;
    }

    return false;
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

  virtual bool sparse_custom_packet(const char* str,res_t& val){return false;}
};

//
//-------------команды-------------------------------------------------------
//


template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_UNID,unid_t>::command_name="UNID";
struct cc_intellitrac_unid : public cc_intellitrac_set_get<CURE_INTELLITRAC_UNID,unid_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_PHONE,phone_t>::command_name="SBAS";
struct cc_intellitrac_phone : public cc_intellitrac_set_get<CURE_INTELLITRAC_PHONE,phone_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac<CURE_MOBITEL_SET_PASSWORD,password>::command_name="CPAS";
struct cc_intellitrac_password : public cc_intellitrac<CURE_MOBITEL_SET_PASSWORD,password>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
};

template<> const char* cc_intellitrac<CURE_INTELLITRAC_VERSION,NullReq,version_t>::command_name="VERSION";
struct cc_intellitrac_version : public cc_intellitrac<CURE_INTELLITRAC_VERSION,NullReq,version_t>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_POWER,pmgr_t>::command_name="PMGR";
struct cc_intellitrac_power : public cc_intellitrac_set_get<CURE_INTELLITRAC_POWER,pmgr_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_INPUT_DELAY,input_delay_t>::command_name="SINP";
struct cc_intellitrac_input_delay : public cc_intellitrac_set_get<CURE_INTELLITRAC_INPUT_DELAY,input_delay_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_VOICE_CALL,voice_call_t>::command_name="VOICE";
struct cc_intellitrac_voice_call : public cc_intellitrac_set_get<CURE_INTELLITRAC_VOICE_CALL,voice_call_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_PIN,pin_t>::command_name="SPIN";
struct cc_intellitrac_pin : public cc_intellitrac_set_get<CURE_INTELLITRAC_PIN,pin_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_GPRS,gprs_t>::command_name="GPRS";
struct cc_intellitrac_gprs : public cc_intellitrac_set_get<CURE_INTELLITRAC_GPRS,gprs_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_intellitrac_track : public icar_custom<CURE_INTELLITRAC_TRACK>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_intellitrac_log : public icar_custom<CURE_INTELLITRAC_LOG>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_intellitrac_receive_log : public virtual icar_custom<CURE_INTELLITRAC_RECEIVE_LOG>,
                                    public icar_custom_self_timeout<CURE_INTELLITRAC_RECEIVE_LOG>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

template<> const char* cc_intellitrac<CURE_INTELLITRAC_OUTS,outs_t>::command_name="OUTS";
struct cc_intellitrac_outs : public cc_intellitrac<CURE_INTELLITRAC_OUTS,outs_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val){return false;}
};

struct cc_intellitrac_reboot : public icar_custom_simplex<CURE_INTELLITRAC_REBOOT>
{bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud); };

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_BATERY,enable_t>::command_name="BBCTRL";
struct cc_intellitrac_batery : public cc_intellitrac_set_get<CURE_INTELLITRAC_BATERY,enable_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac<CURE_INTELLITRAC_VOICE_WIRETAP,phone>::command_name="VMON";
struct cc_intellitrac_wiretap : public cc_intellitrac<CURE_INTELLITRAC_VOICE_WIRETAP,phone>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val){return false;}
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_TIMER,timer_t>::command_name="TIMER";
struct cc_intellitrac_timer : public cc_intellitrac_set_get<CURE_INTELLITRAC_TIMER,timer_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_intellitrac_report : public icar_custom<CURE_INTELLITRAC_REPORT>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_intellitrac_zone : public icar_custom<CURE_INTELLITRAC_ZONE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_SPEED,speed_t>::command_name="SPEEDING";
struct cc_intellitrac_speed : public cc_intellitrac_set_get<CURE_INTELLITRAC_SPEED,speed_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_intellitrac_get_state : public icar_custom<CURE_INTELLITRAC_GET_STATE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_intellitrac_schedule : public icar_custom<CURE_INTELLITRAC_SCHEDULE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

//
// Версия протокола 305
//

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_BAUD,baud_t>::command_name="BAUD";
struct cc_intellitrac_baud : public cc_intellitrac_set_get<CURE_INTELLITRAC_BAUD,baud_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_REPORT_LIFE_CYCLE,life_cycle_t>::command_name="RQLC";
struct cc_intellitrac_report_life_cycle : public cc_intellitrac_set_get<CURE_INTELLITRAC_REPORT_LIFE_CYCLE,life_cycle_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac<CURE_INTELLITRAC_CLEAR_REPORT_QUEUE>::command_name="CLRQ";
struct cc_intellitrac_clear_report_queue : public cc_intellitrac<CURE_INTELLITRAC_CLEAR_REPORT_QUEUE>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val){return false;}
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_GSM_GPRS_SWITCH,gsm_gprs_switch_t>::command_name="GGAS";
struct cc_intellitrac_gsm_gprs_switch : public cc_intellitrac_set_get<CURE_INTELLITRAC_GSM_GPRS_SWITCH,gsm_gprs_switch_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_BACKUP_GPRS,backup_gprs_t>::command_name="BSVR";
struct cc_intellitrac_backup_gprs : public cc_intellitrac_set_get<CURE_INTELLITRAC_BACKUP_GPRS,backup_gprs_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_MILEAGE,mileage_t>::command_name="MILEAGE";
struct cc_intellitrac_mileage : public cc_intellitrac_set_get<CURE_INTELLITRAC_MILEAGE,mileage_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_IDLE_TIMEOUT,idle_timeout_t>::command_name="IDLE";
struct cc_intellitrac_idle_timeout : public cc_intellitrac_set_get<CURE_INTELLITRAC_IDLE_TIMEOUT,idle_timeout_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_ANALOG_REPORT,analog_report_t>::command_name="AREPORT";
struct cc_intellitrac_analog_report : public cc_intellitrac_set_get<CURE_INTELLITRAC_ANALOG_REPORT,analog_report_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac<CURE_INTELLITRAC_GET_UNIT_STATUS,NullReq,unit_status_t>::command_name="QUST";
struct cc_intellitrac_get_unit_status : public cc_intellitrac<CURE_INTELLITRAC_GET_UNIT_STATUS,NullReq,unit_status_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac<CURE_INTELLITRAC_SEND_MDT,mdt_t>::command_name="TMSG";
struct cc_intellitrac_send_mdt : public cc_intellitrac<CURE_INTELLITRAC_SEND_MDT,mdt_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val){return false;}
};

template<> const char* cc_intellitrac_set_get<CURE_TIME_SHIFT,integer_t>::command_name="TZOS";
struct cc_intellitrac_time_shift : public cc_intellitrac_set_get<CURE_TIME_SHIFT,integer_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_universal_command : public icar_custom_packable<CURE_UNIVERSAL_COMMAND,universal_command_t,string_t>
{

  //KIBEZ Добавляем явные определения типов для совместимости с RAD Studio 10.3.3
  typedef universal_command_t req_t;
  typedef string_t res_t;


  bool wait_answer;
  std::string current_command;
  std::string in_set_prefix;
  std::string in_get_prefix;

  cc_universal_command(){wait_answer=false;}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return wait_answer;}
};

}//namespace

#endif

