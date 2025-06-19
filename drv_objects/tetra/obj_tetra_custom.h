#ifndef obj_tetra_customH
#define obj_tetra_customH
#include <pkr_freezer.h>
#include "../device_data/tetra/tetra_commands.h"
#include "../device_data/common_commands.h"

//
//-------------вспомогательные классы----------------------------------------
//
namespace Tetra
{
using namespace VisiPlug::Tetra;
using VisiPlug::enable_t;

struct cc_tetra_track : public icar_custom_packable<CURE_INTELLITRAC_TRACK,enable_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
};

struct cc_tetra_status_report : public icar_custom_packable<CURE_TETRA_STATUS_REPORT,NullReq,status_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_tetra_software_part : public icar_custom_packable<CURE_TETRA_SOFTWARE_PART,NullReq,string_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_tetra_error_report : public icar_custom_packable<CURE_TETRA_ERROR_REPORT,NullReq,interval_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_tetra_track_minute : public icar_custom_packable<CURE_DEDAL_TIME_TRACK,interval_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
};

struct cc_tetra_distance : public icar_custom_packable<CURE_DEDAL_DISTANCE_TRACK,interval_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
};

struct cc_tetra_phone : public icar_custom_packable<CURE_INTELLITRAC_PHONE,phone_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
};

struct cc_fix_kind : public icar_custom_packable<CURE_TETRA_FIX_KIND,interval_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
};

}//namespace

#endif

