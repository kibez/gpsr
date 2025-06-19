#ifndef obj_g200_customH
#define obj_g200_customH

// RAD Studio 10.3.3 compatibility
#ifdef __BORLANDC__
    #if __BORLANDC__ >= 0x0730  // RAD Studio 10.3.3
        #include <System.hpp>
    #endif
#endif

#include <pkr_freezer.h>
#include "../device_data/g200/g200_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace G200
{
using namespace VisiPlug::G200;
class car_g200;

template<int cust_id,class Req=NullReq>
class cc_g200 : public icar_custom_packable<cust_id,Req,NullReq>
{
public:
  //параметры
  typedef typename Req req_t;
public:
  std::string info_header;
  std::string info_footer;
  cc_g200()
  {
    info_header="<INFO ";
    info_footer="OK>";
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
  {
    std::string str;
    if(!sbuild_custom(req,str))return false;
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }

  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
  {
    if(ud.size()<info_header.size()+info_footer.size()||
       !std::equal(info_header.begin(),info_header.end(),ud.begin())||
       !std::equal(info_footer.begin(),info_footer.end(),ud.end()-info_footer.size()))
      return false;

    icar_string_identification& si=dynamic_cast<icar_string_identification&>(*this);

    if(ud.size()<info_header.size()+si.dev_instance_id.size())return false;
    if(!std::equal(si.dev_instance_id.begin(),si.dev_instance_id.end(),ud.begin()+info_header.size()))
      return false;

    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  //«аглушка
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,NullReq& res){return false;}

  virtual bool sbuild_custom(const req_t& req,std::string& str)=0;
};


struct cc_get_state : public icar_custom<CURE_INTELLITRAC_GET_STATE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_gprs : public cc_g200<CURE_EASYTRAC_GPRS,enable_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_gsm : public cc_g200<CURE_EASYTRAC_GSM,enable_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_server : public cc_g200<CURE_EASYTRAC_SERVER,server_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_apn : public cc_g200<CURE_PANTRACK_APN,string_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_track : public cc_g200<CURE_INTELLITRAC_TRACK,track_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_idle_timeout : public cc_g200<CURE_INTELLITRAC_IDLE_TIMEOUT,idle_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_mileage : public icar_custom_packable<CURE_INTELLITRAC_MILEAGE,mileage_t,double_t>
{
  std::string info_header;
  std::string info_footer;
  std::string dsm_header;
  cc_mileage()
  {
    info_header="<INFO ";
    info_footer="OK>";
    dsm_header=",DSM=";
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,double_t& res){return false;}
};

struct cc_resend : public cc_g200<CURE_INTELLITRAC_REPORT_LIFE_CYCLE,integer_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_guard : public cc_g200<CURE_RVCL_ARM,guard_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_input_delay : public cc_g200<CURE_INTELLITRAC_INPUT_DELAY,input_delay_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_outs : public cc_g200<CURE_INTELLITRAC_OUTS,outs_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_input_mask : public cc_g200<CURE_MOBITEL_IN_MASK,integer_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_inputs : public cc_g200<CURE_MOBITEL_ENABLE_IN,input_func_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_analog_inputs : public cc_g200<CURE_INTELLITRAC_ANALOG_REPORT,analog_input_func_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_fix_kind : public cc_g200<CURE_TETRA_FIX_KIND,enable_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_zone : public cc_g200<CURE_INTELLITRAC_ZONE,zone_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_com_cfg : public cc_g200<CURE_INTELLITRAC_BAUD,com_cfg_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_mdt : public cc_g200<CURE_INTELLITRAC_SEND_MDT,mdt_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_time_stamp : public icar_custom_packable<CURE_EASYTRAC_TIME_STAMP,timestamp_t,timestamp_t>
{
  std::string info_header;
  std::string info_footer;
  std::string dsm_header;
  cc_time_stamp()
  {
    info_header="<INFO ";
    info_footer="OK>";
    dsm_header=",RTC=";
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,timestamp_t& res){return false;}
};

struct cc_log : public cc_g200<CURE_INTELLITRAC_LOG,log_t>
{
  std::string footer2;
  cc_log(){footer2=",format complete>";}

  bool sbuild_custom(const req_t& req,std::string& str);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
  {
    if(cc_g200<CURE_INTELLITRAC_LOG,log_t>::iparse_custom_packet(ud,env,packet_id,res))
      return true;
      
    if(ud.size()<info_header.size()+footer2.size()||
       !std::equal(info_header.begin(),info_header.end(),ud.begin())||
       !std::equal(footer2.begin(),footer2.end(),ud.end()-footer2.size()))
      return false;

    icar_string_identification& si=dynamic_cast<icar_string_identification&>(*this);

    if(ud.size()<info_header.size()+si.dev_instance_id.size())return false;
    if(!std::equal(si.dev_instance_id.begin(),si.dev_instance_id.end(),ud.begin()+info_header.size()))
      return false;

    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }
};

struct cc_phone : public cc_g200<CURE_INTELLITRAC_PHONE,phone_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_voice_cfg : public cc_g200<CURE_INTELLITRAC_VOICE_CALL,voice_cfg_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_reboot : public icar_custom_simplex<CURE_INTELLITRAC_REBOOT>
{bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud); };

struct cc_speed : public cc_g200<CURE_INTELLITRAC_SPEED,speed_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_gsm_gprs_switch : public cc_g200<CURE_INTELLITRAC_GSM_GPRS_SWITCH,enable_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_power_save : public cc_g200<CURE_INTELLITRAC_POWER,pwr_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_roaming : public cc_g200<CURE_EASYTRAC_ROAM_LIST,roaming_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_time_zone : public cc_g200<CURE_TIME_SHIFT,integer_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_state : public icar_custom_packable<CURE_INTELLITRAC_GET_UNIT_STATUS,NullReq,state_t>
{
  std::string info_header;
  std::string info_footer;

  std::vector<std::string> buf;
  bool in_progress;

  cc_state()
  {
    info_header="<INFO ";
    info_footer=">";
    in_progress=false;
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_motion_sensor : public cc_g200<CURE_G200_MOTION_SENSOR,motion_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_outs_func : public cc_g200<CURE_G200_OUTPUT_FUNC,out_func_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_analog_output : public cc_g200<CURE_G200_ANALOG_OUTPUT,analog_outs_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_temperature : public cc_g200<CURE_G200_TEMPERATURE_SENSOR,integer_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_img_cfg : public cc_g200<CURE_G200_IMAGE_CFG,img_cfg_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_get_image : public icar_custom<CURE_G200_GET_IMAGE>
{
  unsigned end_time;
  bool in_air;

  cc_get_image()
  {
    end_time=0;
    in_air=false;
  }
  
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);

  void ireset()
  {
    in_air=false;
    end_time=0;
  }

  bool iis_custom_self_timeout() const{return true;}

  bool iis_custom_timeout(unsigned int& timeout) const
  {
    if(!in_air)return timeout==0;
    unsigned cur_time=time(0l);
    if(cur_time>=end_time) return true;
    timeout=end_time-cur_time;
    return false;
  }

};

struct cc_receive_log : public cc_g200<CURE_INTELLITRAC_RECEIVE_LOG,receive_log_t>
{
  std::string info_footer1;

  cc_receive_log(){info_footer1=",OK,";}
  bool sbuild_custom(const req_t& req,std::string& str);
  std::string time_string(time_t stamp);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_factory_settings : public cc_g200<CURE_G200_FACTORY_SETTINGS>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_imei : public cc_g200<CURE_WONDEX_IMEI,enable_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

struct cc_sim : public cc_g200<CURE_WONDEX_SIMID,enable_t>
{
  bool sbuild_custom(const req_t& req,std::string& str);
};

}//namespace

#endif

