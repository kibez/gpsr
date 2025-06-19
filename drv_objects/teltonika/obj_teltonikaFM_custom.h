#ifndef obj_teltonikaFM_customH
#define obj_teltonikaFM_customH
#include <pkr_freezer.h>
#include "../device_data/teltonika/teltonikaFM_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Teltonika
{
using namespace VisiPlug::Teltonika;

struct cc_fm_out : public icar_custom_packable<CURE_INTELLITRAC_OUTS,integer_t,NullReq>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

}//namespace

#endif

