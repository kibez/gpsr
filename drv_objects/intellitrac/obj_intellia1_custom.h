// ============================================================================
// obj_intellia1_custom.h - Мигрировано на RAD Studio 10.3.3
// Пользовательские команды для устройств Intellia1
// ============================================================================
#ifndef obj_intellia1_customH
#define obj_intellia1_customH
#include <System.hpp>
#include <sstream>
#include <pkr_freezer.h>
#include "../device_data/intellitrac/intellia1_commands.h"
#include "../device_data/common_commands.h"

namespace Intellia1
{
using VisiPlug::string_t;
using VisiPlug::universal_command_t;
using VisiPlug::enable_t;
using VisiPlug::integer_t;
using namespace VisiPlug::Intellia1;

struct cc_get_state : public icar_custom<CURE_INTELLITRAC_GET_STATE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_universal_command : public icar_custom_packable<CURE_UNIVERSAL_COMMAND,universal_command_t,string_t>
{
  bool wait_answer;
  bool read_command;
  std::string current_command;
  std::string in_set_prefix;
  std::string in_get_prefix;
  std::string buffered_answer;

  cc_universal_command(){wait_answer=false;read_command=false;}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const std::vector<unsigned char>& ud,const ud_envir& env,res_t& res,custom_result& cres){return false;}
  bool iis_custom_in(const custom_request& req){return wait_answer;}
};

struct cc_reboot : public icar_custom_simplex<CURE_INTELLITRAC_REBOOT>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_imei : public icar_custom_packable<CURE_WONDEX_IMEI,enable_t,string_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const std::vector<unsigned char>& ud,const ud_envir& env,res_t& res,custom_result& cres);
};

struct cc_wiretap : public icar_custom_packable<CURE_INTELLITRAC_VOICE_WIRETAP,string_t,string_t>
{
  bool read_command;
  std::string in_set_prefix;
  std::string in_get_prefix;

  cc_wiretap(){read_command=true;}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const std::vector<unsigned char>& ud,const ud_envir& env,res_t& res,custom_result& cres);
};

}
#endif
