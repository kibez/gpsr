#ifndef obj_easytrac_customH
#define obj_easytrac_customH
#include <pkr_freezer.h>
#include "../device_data/easytrac/easytrac_commands.h"
#include "../device_data/mobitel/mobitel_commands.h"
#include "../device_data/common_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace EasyTrac
{
using namespace VisiPlug::EasyTrac;
using VisiPlug::Mobitel::password;
using VisiPlug::enable_t;

template<int cust_id,class Req>
class cc_easytrac_set_get : public icar_custom_packable<cust_id,Req,Req>
{
public:
  typedef cc_easytrac_set_get<cust_id,Req> cmd_parent_t;
  //параметры
  typedef typename Req req_t;
  //результат
  typedef typename Req res_t;
protected:
  std::string command_name;
  bool is_act;

  std::string set_prefix;
  std::string get_prefix;
public:
  cc_easytrac_set_get(const std::string& _command_name,bool _is_act=false){command_name=_command_name;is_act=_is_act;}

  void init(icar_custom_ctrl* d)
  {
    icar_custom<cust_id>::init(d);
    if(is_act)
    {
      set_prefix="$ActSet"+command_name+",";
      get_prefix="$ActGet"+command_name+",";
    }
    else
    {
      set_prefix="$Set"+command_name+",";
      get_prefix="$Get"+command_name+",";
    }
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
  {
    std::string str;
    if(req.set)str=set_prefix;
    else str=get_prefix;

    str+=dynamic_cast<car_easytrac*>(this)->header_str(packet_id);

    if(req.set&&!sbuild_custom_packet(req,str))
      return false;

    str+="&";
    ud.insert(ud.end(),str.begin(),str.end() );
    return true;
  }

  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
  {
    bool is_set;

    if(ud.size()>set_prefix.size()&&std::equal(set_prefix.begin(),set_prefix.end(),ud.begin())) is_set=true;
    else if(ud.size()>get_prefix.size()&&std::equal(get_prefix.begin(),get_prefix.end(),ud.begin()))is_set=false;
    else return false;

    std::string str;
    if(is_set)str.append(ud.begin()+set_prefix.size(),ud.end()-1 );
    else str.append(ud.begin()+get_prefix.size(),ud.end()-1 );

    packet_id.use_packet_identificator=true;
    if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
    size_t ps=str.find(',');
    if(ps==str.npos)return false;
    str.erase(str.begin(),str.begin()+ps+1);

    ps=str.rfind(',');
    if(ps!=str.npos)str.erase(str.begin()+ps,str.end());

    res.res_mask|=CUSTOM_RESULT_END;

    if(is_set)
    {
      if(str=="OK")res.err_code=POLL_ERROR_NO_ERROR;
      else if(str=="Cmd")res.err_code=ERROR_OBJECT_NOT_SUPPORT;
      else if(str=="Pwd")res.err_code=ERROR_OPERATION_DENIED;
      else res.err_code=ERROR_OPERATION_FAILED;

      return true;
    }

    res_t v;
    v.set=false;

    if(str=="Cmd")res.err_code=ERROR_OBJECT_NOT_SUPPORT;
    else if(str=="Pwd")res.err_code=ERROR_OPERATION_DENIED;
    else if(str=="Param")res.err_code=ERROR_OPERATION_FAILED;
    else if(sparse_custom_packet(str,v))
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

  virtual bool sbuild_custom_packet(const req_t& req,std::string& str)=0l;
  virtual bool sparse_custom_packet(const std::string& str,res_t& val)=0l;
};


template<int cust_id,class Req=NullReq,class Res=NullReq>
class cc_easytrac : public icar_custom_packable<cust_id,Req,Res>
{
public:
  typedef cc_easytrac<cust_id,Req,Res> cmd_parent_t;
  //параметры
  typedef typename Req req_t;
  //результат
  typedef typename Res res_t;
protected:
  std::string command_name;
public:
  cc_easytrac(const std::string& _command_name)
  {
    command_name=_command_name+",";
  }

  void init(icar_custom_ctrl* d)
  {
    icar_custom<cust_id>::init(d);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
  {
    std::string str=command_name;
    str+=dynamic_cast<car_easytrac*>(this)->header_str(packet_id);

    if(!sbuild_custom_packet(req,str))
      return false;

    str+="&";
    ud.insert(ud.end(),str.begin(),str.end() );
    return true;
  }

  virtual bool sbuild_custom_packet(const req_t& req,std::string& str)=0;

  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
  {
    return int_iparse_custom_packet(ud,env,packet_id,res,t2t<Res>());
  }

  template<class T>
  bool int_iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res,t2t<T>)
  {
    if(ud.size()<=command_name.size()||!std::equal(command_name.begin(),command_name.end(),ud.begin()))
      return false;

    std::string str;
    str.append(ud.begin()+command_name.size(),ud.end()-1 );

    packet_id.use_packet_identificator=true;
    if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
    size_t ps=str.find(',');
    if(ps==str.npos)return false;
    str.erase(str.begin(),str.begin()+ps+1);

    ps=str.rfind(',');
    if(ps!=str.npos)str.erase(str.begin()+ps,str.end());
    

    res.res_mask|=CUSTOM_RESULT_END;

    res_t v;

    if(str=="Cmd")res.err_code=ERROR_OBJECT_NOT_SUPPORT;
    else if(str=="Pwd")res.err_code=ERROR_OPERATION_DENIED;
    else if(str=="Param")res.err_code=ERROR_OPERATION_FAILED;
    else if(sparse_custom_packet(str,v))
    {
      res.res_mask|=CUSTOM_RESULT_DATA;
      pkr_freezer fr(v.pack());
      res.set_data(fr.get());
    }
    else res.err_code=POLL_ERROR_PARSE_ERROR;
    return true;
  }

  bool int_iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res,t2t<NullReq>)
  {
    if(ud.size()<=command_name.size()||!std::equal(command_name.begin(),command_name.end(),ud.begin()))
      return false;

    std::string str;
    str.append(ud.begin()+command_name.size(),ud.end()-1 );

    packet_id.use_packet_identificator=true;
    if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
    size_t ps=str.find(',');
    if(ps==str.npos)return false;
    str.erase(str.begin(),str.begin()+ps+1);

    ps=str.rfind(',');
    if(ps!=str.npos)str.erase(str.begin()+ps,str.end());

    res.res_mask|=CUSTOM_RESULT_END;

    if(str=="OK")res.err_code=POLL_ERROR_NO_ERROR;
    else if(str=="Cmd")res.err_code=ERROR_OBJECT_NOT_SUPPORT;
    else if(str=="Pwd")res.err_code=ERROR_OPERATION_DENIED;
    else res.err_code=ERROR_OPERATION_FAILED;

    return true;
  }

  //Заглушка
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}

  virtual bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

//
//-------------команды-------------------------------------------------------
//

struct cc_easytrac_serial_no : public cc_easytrac<CURE_EASYTRAC_SERIAL,NullReq,serial_t>
{
  cc_easytrac_serial_no() : cmd_parent_t("$GetSerialNo"){}

  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_unid : public cc_easytrac_set_get<CURE_EASYTRAC_UNID,unit_t>
{
  cc_easytrac_unid() : cmd_parent_t("UnitID"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_password : public cc_easytrac<CURE_MOBITEL_SET_PASSWORD,password>
{
  cc_easytrac_password() : cmd_parent_t("$SetPwd"){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
};

struct cc_easytrac_simpin : public cc_easytrac_set_get<CURE_EASYTRAC_SIMPIN,sim_t>
{
  cc_easytrac_simpin() : cmd_parent_t("SIMPINx"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_switch_mode : public cc_easytrac_set_get<CURE_EASYTRAC_SWITCH_MODE,switch_mode_t>
{
  cc_easytrac_switch_mode() : cmd_parent_t("SwitchMode"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_gprs_config : public cc_easytrac_set_get<CURE_EASYTRAC_GPRS,gprs_t>
{
  cc_easytrac_gprs_config() : cmd_parent_t("GPRSConfig"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);

  static std::string mode2str(TrackMode md);
  static TrackMode str2mode(const std::string& val);
};

struct cc_easytrac_resync : public cc_easytrac_set_get<CURE_EASYTRAC_RESYNC,resync_t>
{
  cc_easytrac_resync() : cmd_parent_t("Resync"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_gsm : public cc_easytrac_set_get<CURE_EASYTRAC_GSM,gsm_t>
{
  cc_easytrac_gsm() : cmd_parent_t("Connection"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_server : public cc_easytrac_set_get<CURE_EASYTRAC_SERVER,server_t>
{
  cc_easytrac_server() : cmd_parent_t("PriServer"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_gsmconfig : public cc_easytrac_set_get<CURE_EASYTRAC_GSM_CONFIG,sms_t>
{
  cc_easytrac_gsmconfig() : cmd_parent_t("GSMConfig"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_time_stamp : public cc_easytrac_set_get<CURE_EASYTRAC_TIME_STAMP,timestamp_t>
{
  cc_easytrac_time_stamp() : cmd_parent_t("TimeStamp"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_time_shift : public cc_easytrac_set_get<CURE_TIME_SHIFT,timeshift_t>
{
  cc_easytrac_time_shift() : cmd_parent_t("TimeZone"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_sms_tel : public cc_easytrac_set_get<CURE_EASYTRAC_SMS_TEL,sms_tel_t>
{
  cc_easytrac_sms_tel() : cmd_parent_t("SMSTel"){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_roam_list : public cc_easytrac_set_get<CURE_EASYTRAC_ROAM_LIST,roam_list_t>
{
  cc_easytrac_roam_list() : cmd_parent_t("RoamList"){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_gprs_track : public cc_easytrac<CURE_EASYTRAC_GPRS_TRACK,gprs_track_t>
{
  cc_easytrac_gprs_track() : cmd_parent_t("$ActGPRSTrack"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_get_log : public cc_easytrac<CURE_EASYTRAC_LOG,log_t>
{
  cc_easytrac_get_log() : cmd_parent_t("$ActGetLog"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_save_cfg : public cc_easytrac<CURE_EASYTRAC_SAVE_CFG>
{
  cc_easytrac_save_cfg() : cmd_parent_t("$ActSaveCFG"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_save_var_cfg : public cc_easytrac<CURE_EASYTRAC_SAVE_VAR_CFG>
{
  cc_easytrac_save_var_cfg() : cmd_parent_t("$ActSaveVarCFG"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_reboot : public cc_easytrac<CURE_INTELLITRAC_REBOOT>
{
  cc_easytrac_reboot() : cmd_parent_t("$ActReboot"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_output : public cc_easytrac<CURE_EASYTRAC_OUT,outs_t>
{
  cc_easytrac_output() : cmd_parent_t("$ActSetDigitalOut"){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_easytrac_mile : public cc_easytrac_set_get<CURE_EASYTRAC_MILE,mileage_t>
{
  cc_easytrac_mile() : cmd_parent_t("TotalMile",true){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_mile_limit : public cc_easytrac_set_get<CURE_EASYTRAC_MILE_LIMIT,mileage_t>
{
  cc_easytrac_mile_limit() : cmd_parent_t("MileLimit",true){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_distance : public cc_easytrac_set_get<CURE_EASYTRAC_DISTANCE,mileage_t>
{
  cc_easytrac_distance() : cmd_parent_t("Distance",true){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_angle_track : public cc_easytrac_set_get<CURE_EASYTRAC_ANGLE_TRACK,mileage_t>
{
  cc_easytrac_angle_track() : cmd_parent_t("DevAng",true){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_idle : public cc_easytrac_set_get<CURE_EASYTRAC_IDLE,idle_timeout_t>
{
  cc_easytrac_idle() : cmd_parent_t("Idle",true){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_speed : public cc_easytrac_set_get<CURE_EASYTRAC_SPEED,speed_t>
{
  cc_easytrac_speed() : cmd_parent_t("Speed",true){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_fence : public cc_easytrac_set_get<CURE_EASYTRAC_PARK_FENCE,fence_t>
{
  cc_easytrac_fence() : cmd_parent_t("ParkFence",true){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_zone : public cc_easytrac_set_get<CURE_EASYTRAC_ZONE,zone_t>
{
  cc_easytrac_zone() : cmd_parent_t("GeoFence",true){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
  bool sparse_custom_packet(const std::string& str,res_t& val);

  static void degree2parts(double val,double& d,double& m,double& s);
  static double parts2degree(double d,double m,double s);
};

struct cc_easytrac_state : public cc_easytrac<CURE_EASYTRAC_GET_STATE,NullReq,common_t>
{
  std::string result_message;
  cc_easytrac_state() : cmd_parent_t("$ActGetPos"){result_message="$LGPS";}
  bool sbuild_custom_packet(const req_t& req,std::string& str){str+=",YES";return true;}
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool isame_in_air(const custom_request& rq,const custom_request& air_rq){return true;}
};

struct cc_easytrac_gsm_track : public cc_easytrac<CURE_EASYTRAC_GSM_TRACK,sms_t>
{
  cc_easytrac_gsm_track() : cmd_parent_t("$ActGSMTrack"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val){return true;}
};

struct cc_easytrac_clear_log : public cc_easytrac<CURE_INTELLITRAC_CLEAR_REPORT_QUEUE>
{
  cc_easytrac_clear_log() : cmd_parent_t("$ActClearLog"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_baud : public cc_easytrac_set_get<CURE_INTELLITRAC_BAUD,baud_t>
{
  cc_easytrac_baud() : cmd_parent_t("COM"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_rfid : public cc_easytrac_set_get<CURE_EASYTRAC_RFID,id_control_t>
{
  cc_easytrac_rfid() : cmd_parent_t("IDControl"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_rfid_list : public cc_easytrac_set_get<CURE_EASYTRAC_RFID_LIST,rfid_list_t>
{
  cc_easytrac_rfid_list() : cmd_parent_t("ValidID"){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_update_firmware : public icar_custom_packable<CURE_EASYTRAC_UPDATE_FIRMWARE,serial_t>
{
  std::string prefix;
  cc_easytrac_update_firmware(){prefix="$ActUpgradeFW,";}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
};

struct cc_easytrac_mdt : public cc_easytrac<CURE_INTELLITRAC_SEND_MDT,mdt_t>
{
  cc_easytrac_mdt() : cmd_parent_t("$ActComOut"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_event_mask : public cc_easytrac_set_get<CURE_EVENTS_MASK,event_mask_t>
{
  cc_easytrac_event_mask() : cmd_parent_t("EventMask"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_odb_limit : public cc_easytrac_set_get<CURE_EASYTRAC_ENGINE_TEMP,odb_limit_t>
{
  cc_easytrac_odb_limit() : cmd_parent_t("OBDlimit"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_atr : public cc_easytrac<CURE_EASYTRAC_ATR,mileage_t>
{
  cc_easytrac_atr() : cmd_parent_t("$ATR"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_temp_limit : public cc_easytrac_set_get<CURE_G200_TEMPERATURE_SENSOR,temp_limit_t>
{
  cc_easytrac_temp_limit() : cmd_parent_t("TempLimit"){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_power : public cc_easytrac_set_get<CURE_INTELLITRAC_POWER,pwm_t>
{
  cc_easytrac_power() : cmd_parent_t("PowerConfig"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_binary_mode : public cc_easytrac_set_get<CURE_EASYTRAC_BINARY_MODE,enable_t>
{
  cc_easytrac_binary_mode() : cmd_parent_t("DataMode"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_analog_mode : public cc_easytrac_set_get<CURE_INTELLITRAC_ANALOG_REPORT,analog_mode_t>
{
  cc_easytrac_analog_mode() : cmd_parent_t("AIMode"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val);
};

struct cc_easytrac_send_sms : public cc_easytrac<CURE_EASYTRAC_SEND_SMS,send_sms_t>
{
  cc_easytrac_send_sms() : cmd_parent_t("$ActSendSMS"){}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const std::string& str,res_t& val){return false;}
};

struct cc_easytrac_spd_and_dist : public icar_custom_packable<CURE_EASYTRAC_SPD_AND_DIST, spd_and_dist_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res) {return false;}
};

}//namespace

#endif

