#ifndef obj_fenix_customH
#define obj_fenix_customH
#include <pkr_freezer.h>
#include "../device_data/fenix/fenix_commands.h"
#include "../device_data/mobitel/mobitel_commands.h"
#include "../device_data/common_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Fenix
{
using namespace VisiPlug::Fenix;
using VisiPlug::Mobitel::password;
using VisiPlug::enable_t;

struct cc_get_state : public icar_custom<CURE_INTELLITRAC_GET_STATE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_password : public icar_custom_packable<CURE_MOBITEL_SET_PASSWORD,password>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_sim : public icar_custom_packable<CURE_WONDEX_SIMID,sim_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_cfg : public icar_custom_packable<CURE_INTELLIX1_PARAMS,cfg_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_alert : public icar_custom_packable<CURE_TELTONIKA_ALERT>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_off : public icar_custom_packable<CURE_MOBITEL_OFF>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_owner : public icar_custom_packable<CURE_MOBITEL_SET_VIEWER_NUMBER>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_sms : public icar_custom_packable<CURE_INTELLITRAC_TRACK,enable_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_gps_on : public icar_custom_packable<CURE_FENIX_GPS_ON,enable_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_gps : public icar_custom_packable<CURE_FENIX_GPS,minutes_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_gsm : public icar_custom_packable<CURE_EASYTRAC_GSM,minutes_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_sleep : public icar_custom_packable<CURE_FENIX_SLEEP,pwm_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

struct cc_gsm_info : public icar_custom_packable<CURE_WONDEX_GSM_INFO,NullReq,gsm_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
  static bool sparse_gsm_info(const data_t& ud,res_t& res);
};

}//namespace

#endif

