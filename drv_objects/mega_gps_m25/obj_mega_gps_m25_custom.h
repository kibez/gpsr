#ifndef obj_mega_gsp_m25_customH
#define obj_mega_gsp_m25_customH
#include <pkr_freezer.h>
#include "../device_data/mega_gps_m25/mega_gps_m25_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace MegaGps
{
using namespace VisiPlug::MegaGps;

class cc_outs : public icar_custom_packable<CURE_MEGA_GPS_OUT,VisiPlug::enable_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
  //============
  data_t cco_id;    //tracker_id for requests
};

class cc_set_timeouts : public icar_custom_packable<CURE_MEGA_GPS_SET_TIMEOUTS,set_timeouts_t>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
  //============
  data_t ccst_id;      //tracker_id for requests
};

class cc_sw_update : public icar_custom_packable<CURE_MEGA_GPS_SW_UPDATE>
{
public:
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return false;}
  //===============
  data_t ccsu_id;        //tracker_id for requests
};

}//namespace

#endif

