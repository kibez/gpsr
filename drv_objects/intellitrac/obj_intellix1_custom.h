#ifndef obj_intellix1_customH
#define obj_intellix1_customH
#include <pkr_freezer.h>
#include "../device_data/intellix1/intellix1_commands.h"
#include "../device_data/intellitrac/intellitrac_commands.h"
#include "../device_data/mobitel/mobitel_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Intellix1
{

using namespace VisiPlug::Intellix1;
using Intellitrac::cc_intellitrac_set_get;
using Intellitrac::cc_intellitrac;

template<> const char* cc_intellitrac_set_get<CURE_INTELLIX1_PARAMS,unit_params_t>::command_name="UNPM";
struct cc_intellix1_params : public cc_intellitrac_set_get<CURE_INTELLIX1_PARAMS,unit_params_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLIX1_COMMUNICATION,communication_t>::command_name="COMM";
struct cc_intellix1_communication : public cc_intellitrac_set_get<CURE_INTELLIX1_COMMUNICATION,communication_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_intellix1_track : public icar_custom<CURE_INTELLITRAC_TRACK>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLIX1_REPORT_MASK,report_mask_t>::command_name="RMSK";
struct cc_intellix1_report_mask : public cc_intellitrac_set_get<CURE_INTELLIX1_REPORT_MASK,report_mask_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_intellix1_report : public icar_custom<CURE_INTELLITRAC_REPORT>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_intellix1_clear_user_reports : public icar_custom_simplex<CURE_INTELLIX1_CLEAR_USER_REPORTS>
{bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud); };

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_POWER,pmgr_t>::command_name="PMGR";
struct cc_intellix1_power : public cc_intellitrac_set_get<CURE_INTELLITRAC_POWER,pmgr_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLIX1_TOW,tow_t>::command_name="TOW";
struct cc_intellix1_tow : public cc_intellitrac_set_get<CURE_INTELLIX1_TOW,tow_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

template<> const char* cc_intellitrac_set_get<CURE_INTELLITRAC_MILEAGE,mileage_t>::command_name="MILE";
struct cc_intellix1_mileage : public cc_intellitrac_set_get<CURE_INTELLITRAC_MILEAGE,mileage_t>
{
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_intellix1_upgrade : public virtual icar_custom<CURE_INTELLIX1_UPGRADE>,
                                    public icar_custom_self_timeout<CURE_INTELLIX1_UPGRADE>
{
  std::string data;
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_intellix1_speed : public icar_custom<CURE_INTELLITRAC_SPEED>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

}//namespace

#endif

