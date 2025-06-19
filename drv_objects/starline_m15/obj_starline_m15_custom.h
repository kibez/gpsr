#ifndef obj_starline_m15_customH
#define obj_starline_m15_customH

#include <pkr_freezer.h>
#include "../device_data/starline_m15/starline_m15_commands.h"

namespace StarLine_M15
{

using namespace VisiPlug::StarLine_M15;

class cc_owner_num : public icar_custom_packable<CURE_PHANTOM_SET_BASE_GSM_NUMBER, owner_num_t, owner_num_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_date_time : public icar_custom_packable<CURE_STARLINE_M15_DATETIME, date_time_t, date_time_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_timers : public icar_custom_packable<CURE_PHANTOM_SET_TIMER, timers_t, timers_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_motion_sensor : public icar_custom_packable<CURE_G200_MOTION_SENSOR, motion_sensor_t, motion_sensor_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_params : public icar_custom_packable<CURE_PHANTOM_GET_ALL_DEVICE_SETTINGS>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_get_status : public icar_custom_packable<CURE_INTELLITRAC_GET_STATE, state_mode_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_state_mode : public icar_custom_packable<CURE_STARLINE_M15_STATE_MODE, state_mode_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_update_firmware : public icar_custom_packable<CURE_EASYTRAC_UPDATE_FIRMWARE>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_mon : public icar_custom_packable<CURE_STARLINE_M15_MON, mon_t, mon_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_password : public icar_custom_packable<CURE_MOBITEL_SET_PASSWORD, password_t, password_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_info : public icar_custom_packable<CURE_STARLINE_M15_INFO>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

}

#endif
