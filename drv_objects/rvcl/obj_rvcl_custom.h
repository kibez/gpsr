#ifndef obj_rvcl_customH
#define obj_rvcl_customH
#include <pkr_freezer.h>
#include "../device_data/rvcl/rvcl_commands.h"
#include "../device_data/common_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Rvcl
{
using namespace VisiPlug::Rvcl;
using VisiPlug::enable_true_t;

//
//-------------команды-------------------------------------------------------
//

struct cc_get_state : public icar_custom_packable<CURE_RVCL_GET_STATE,NullReq,state_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_shut_down : public icar_custom_simplex<CURE_RVCL_SHUT_DOWN>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_blinker : public icar_custom_packable<CURE_RVCL_BLINKER,enable_true_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_arm : public icar_custom_packable<CURE_RVCL_ARM,enable_true_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_block : public icar_custom_packable<CURE_RVCL_BLOCK,enable_true_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_lock : public icar_custom_packable<CURE_RVCL_LOCK,enable_true_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_siren : public icar_custom_packable<CURE_RVCL_SIREN,enable_true_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_service : public icar_custom_packable<CURE_RVCL_SERVICE,enable_true_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_trunk : public icar_custom<CURE_RVCL_TRUNK>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_panic : public icar_custom<CURE_RVCL_PANIC>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_mute : public icar_custom_packable<CURE_RVCL_MUTE,enable_true_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};


}//namespace

#endif

