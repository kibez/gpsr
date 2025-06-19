#ifndef obj_bitrek_customH
#define obj_bitrek_customH
#include <pkr_freezer.h>
#include "../device_data/bitrek/bitrek_commands.h"

namespace Bitrek
{

using namespace VisiPlug::Bitrek;

struct cc_bireset : public icar_custom_simplex<CURE_PANTRACK_RESET>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud);
};

struct cc_biversion : public icar_custom_packable<CURE_INTELLITRAC_VERSION, NullReq, version_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_bigetgps : public icar_custom_packable<CURE_INTELLITRAC_TRACK, NullReq, track_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_bidelgps : public icar_custom_packable<CURE_PANTRACK_RESET_LOG, NullReq, delgps_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_bigetio : public icar_custom_packable<CURE_PANTRACK_GET_STATE, NullReq, io_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_bisetout : public icar_custom_packable<CURE_INTELLITRAC_OUTS, setouts_t, setouts_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_bifwupdate : public icar_custom_packable<CURE_EASYTRAC_UPDATE_FIRMWARE, fwupdate_t, fwupdate_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_bigetparam : public icar_custom_packable<CURE_INTELLIX1_PARAMS, params_t, params_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

}

#endif
 