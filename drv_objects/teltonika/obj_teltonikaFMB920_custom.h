
#ifndef obj_teltonikaFMB920_customH
#define obj_teltonikaFMB920_customH

#include "../device_data/teltonikaFMB920/teltonikaFMB920_commands.h"

namespace Teltonika
{
using namespace VisiPlug::Teltonika;

struct cc_fm_out_fmb920 : public icar_custom_packable<CURE_INTELLITRAC_OUTS,fmb920_out_t,NullReq>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_params_fmb920 : public icar_custom_packable<CURE_INTELLIX1_PARAMS,fmb920_params_t,fmb920_params_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

}

#endif