#ifndef obj_nd_customH
#define obj_nd_customH
#include <pkr_freezer.h>
#include "../device_data/intellitrac/intellitrac_commands.h"
#include "../device_data/nd/nd_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Nd
{
using Intellitrac::cc_intellitrac;
using Intellitrac::cc_intellitrac_set_get;
using VisiPlug::Intellitrac::mdt_t;
using namespace VisiPlug::Nd;
using VisiPlug::Intellitrac::backup_gprs_t;


template<> const char* cc_intellitrac<CURE_ND_AT_COMMAND,mdt_t,mdt_t>::command_name="ATCOMMAND";
struct cc_at_command : public cc_intellitrac<CURE_ND_AT_COMMAND,mdt_t,mdt_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_track : public icar_custom<CURE_INTELLITRAC_TRACK>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_imei : public icar_custom<CURE_WONDEX_IMEI>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};


template<> const char* cc_intellitrac_set_get<CURE_ND_TRAFF,traff_t>::command_name="TRAFF";
struct cc_traff : public cc_intellitrac_set_get<CURE_ND_TRAFF,traff_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_ND_SPIN,sim_conf_t>::command_name="SPIN";
struct cc_sim_conf : public cc_intellitrac_set_get<CURE_ND_SPIN,sim_conf_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_ND_UPDATE_SERVER,update_server_t>::command_name="CASL";
struct cc_update_server : public cc_intellitrac_set_get<CURE_ND_UPDATE_SERVER,update_server_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_ND_AUTOCMD,auto_cmd_t>::command_name="AUTOCMD";
struct cc_auto_cmd : public cc_intellitrac_set_get<CURE_ND_AUTOCMD,auto_cmd_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_ND_STOP_FILTER,stopped_filter_t>::command_name="STOPPEDFILTR";
struct cc_stopped_filter : public cc_intellitrac_set_get<CURE_ND_STOP_FILTER,stopped_filter_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac<CURE_EASYTRAC_SEND_SMS,send_sms_t>::command_name="SMS";
struct cc_send_sms : public cc_intellitrac<CURE_EASYTRAC_SEND_SMS,send_sms_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val){return false;}
};

template<> const char* cc_intellitrac_set_get<CURE_ND_GSMCONF,string_t>::command_name="GSMCONF";
struct cc_gsm_conf : public cc_intellitrac_set_get<CURE_ND_GSMCONF,string_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac<CURE_ND_DEVSTATE,NullReq,dev_state_t>::command_name="DEVSTATE";
struct cc_dev_state : public cc_intellitrac<CURE_ND_DEVSTATE,NullReq,dev_state_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str){return true;}
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_roaming : public icar_custom<CURE_ND_ROAMING>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_mdt : public icar_custom<CURE_INTELLITRAC_SEND_MDT>
{
  unsigned current_id;
  bool ok_received;
  cc_mdt(){current_id=0;ok_received=false;}
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_off : public icar_custom<CURE_MOBITEL_OFF>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_update_fw : public icar_custom<CURE_EASYTRAC_UPDATE_FIRMWARE>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

}//namespace

#endif

