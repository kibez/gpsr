#ifndef obj_phantom_customH
#define obj_phantom_customH
#include <pkr_freezer.h>
#include "../device_data/phantom/phantom_commands.h"
#include "../device_data/phantom/phantom_notify.h"
#include "../device_data/common_commands.h"

namespace Phantom
{

using namespace VisiPlug::Phantom;

class cc_set_timer : public icar_custom_packable<CURE_PHANTOM_SET_TIMER, set_timer_t, set_timer_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_set_base_gsm_num : public icar_custom_packable<CURE_PHANTOM_SET_BASE_GSM_NUMBER, telnum_t, telnum_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_get_all_settings : public icar_custom_packable<CURE_PHANTOM_GET_ALL_DEVICE_SETTINGS, all_settings_t, all_settings_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

class cc_set_btr_lvl : public icar_custom_packable<CURE_PHANTOM_SET_BTR_LVL, biasi_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res){return false;};
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
};

}

#endif