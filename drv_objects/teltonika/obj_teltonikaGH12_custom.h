#ifndef obj_teltonikaGH12_customH
#define obj_teltonikaGH12_customH
#include <pkr_freezer.h>
#include "../device_data/teltonika/teltonikaGH12_commands.h"
#include "../device_data/common_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Teltonika
{
using namespace VisiPlug::Teltonika;
using VisiPlug::enable_true_t;

struct cc_get_state : public icar_custom<CURE_INTELLITRAC_GET_STATE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_alarm : public icar_custom_packable<CURE_TELTONIKA_ALERT,enable_true_t,common_t>
{
  bool on_state;
  cc_alarm(){on_state=false;}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_track : public icar_custom_packable<CURE_INTELLITRAC_TRACK,track_t,common_t>
{
  bool on_state;
  cc_track(){on_state=false;}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_inf : public icar_custom_packable<CURE_INTELLITRAC_GET_UNIT_STATUS,NullReq,inf_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_geo : public icar_custom_packable<CURE_INTELLITRAC_ZONE,NullReq,zone_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_life : public icar_custom_packable<CURE_TELTONIKA_UPTIME,NullReq,common_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_log : public icar_custom_simplex<CURE_INTELLITRAC_RECEIVE_LOG>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud);
};

struct cc_reset : public icar_custom_simplex<CURE_PANTRACK_RESET>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud);
};

struct cc_power_off : public icar_custom_simplex<CURE_MOBITEL_OFF>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud);
};

}//namespace

#endif

