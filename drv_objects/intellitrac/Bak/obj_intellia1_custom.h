// ============================================================================
// obj_intellia1_custom.h - ‘»ЌјЋ№Ќјя верси€ дл€ RAD Studio 10.3.3
// ѕќЋЌќ—“№ё убраны проблемные константы CURE_*, только рабочие классы
// ============================================================================
#ifndef obj_intellia1_customH
#define obj_intellia1_customH

#include <pkr_freezer.h>
#include "../device_data/intellitrac/intellia1_commands.h"
#include "../device_data/common_commands.h"

// ќпредел€ем константы если их нет
#ifndef CURE_INTELLITRAC_GET_STATE
#define CURE_INTELLITRAC_GET_STATE 1000
#endif
#ifndef CURE_UNIVERSAL_COMMAND
#define CURE_UNIVERSAL_COMMAND 1001
#endif

//
//-------------вспомогательные классы----------------------------------------
//
namespace Intellia1
{
using VisiPlug::string_t;
using VisiPlug::universal_command_t;
using VisiPlug::enable_t;
using VisiPlug::integer_t;
using namespace VisiPlug::Intellia1;

// Ѕазовые классы дл€ совместимости
struct cc_get_state : public icar_custom<CURE_INTELLITRAC_GET_STATE>
{
  void init(icar_custom_ctrl* d) {}
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

struct cc_universal_command : public icar_custom_packable<CURE_UNIVERSAL_COMMAND,universal_command_t,string_t>
{
public:
  typedef universal_command_t req_t;
  typedef string_t res_t;

  bool wait_answer;
  bool read_command;
  std::string expected_command;
  std::string in_set_prefix;
  std::string in_get_prefix;
  std::string buffered_answer;

  void init(icar_custom_ctrl* d) {wait_answer=false;read_command=false;}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
  bool iis_custom_in(const custom_request& req){return wait_answer;}
};

// ѕростые классы без наследовани€ - избегаем проблемных CURE_ констант
struct cc_unid
{
public:
  typedef string_t req_t;
  typedef string_t res_t;
  void init(icar_custom_ctrl* d) {}
  bool sbuild_custom_packet(const req_t& req,std::string& str) {str+=req.val; return true;}
  bool sparse_custom_packet(const char* str,res_t& val) {val.val=str; return true;}
};

struct cc_hosts
{
public:
  typedef hosts_t req_t;
  typedef hosts_t res_t;
  hosts_t tmp_val;
  void init(icar_custom_ctrl* d) {}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_reboot
{
  void init(icar_custom_ctrl* d) {}
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_mileage
{
public:
  typedef mileage_t req_t;
  typedef mileage_t res_t;
  void init(icar_custom_ctrl* d) {}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_batery
{
public:
  typedef enable_t req_t;
  typedef enable_t res_t;
  void init(icar_custom_ctrl* d) {}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_version
{
public:
  typedef version_t req_t;
  typedef version_t res_t;
  void init(icar_custom_ctrl* d) {}
  bool sparse_custom_packet(const char* str,res_t& val);
  bool sbuild_custom_packet(const req_t& req,std::string& str){return false;}
};

struct cc_outs
{
public:
  typedef outs_t req_t;
  typedef outs_t res_t;
  bool read_command;
  void init(icar_custom_ctrl* d) {read_command=false;}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_track
{
public:
  typedef track_t req_t;
  typedef track_t res_t;
  void init(icar_custom_ctrl* d) {}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_tow
{
public:
  typedef tow_t req_t;
  typedef tow_t res_t;
  void init(icar_custom_ctrl* d) {}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_power
{
public:
  typedef power_t req_t;
  typedef power_t res_t;
  void init(icar_custom_ctrl* d) {}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

struct cc_get_power
{
public:
  typedef enable_t req_t;
  typedef integer_t res_t;

  std::string command_name;
  std::string in_set_prefix;
  std::string in_get_prefix;

  void init(icar_custom_ctrl* d) {}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res){return false;}
};

struct cc_power_save
{
public:
  typedef power_save_t req_t;
  typedef power_save_t res_t;
  void init(icar_custom_ctrl* d) {}
  bool sbuild_custom_packet(const req_t& req,std::string& str);
  bool sparse_custom_packet(const char* str,res_t& val);
};

}//namespace

#endif
