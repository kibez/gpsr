#ifndef obj_wondex_conditionH
#define obj_wondex_conditionH
#include "../device_data/wondex/wondex_notify.h"
#include "../device_data/wondex/wondex_commands.h"

namespace Wondex
{

using namespace VisiPlug::Wondex;

class car_wondex;

struct co_common : public icar_condition<0>
{
  car_wondex& parent;
  co_common(car_wondex& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_synchro_packet : public icar_condition<CONDITION_INTELLITRAC_SYNCHRO_PACKET>
{
  car_wondex& parent;
  co_synchro_packet(car_wondex& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_mdt : public icar_condition<CONDITION_INTELLITRAC_MDT>
{
  car_wondex& parent;
  co_mdt(car_wondex& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_gsm_location : public icar_condition<CONDITION_WONDEX_GSM_LOCATION>
{
  car_wondex& parent;
  co_gsm_location(car_wondex& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_mgsm_location : public icar_condition<CONDITION_WONDEX_MULTI_GSM_LOCATION>
{
  car_wondex& parent;
  co_mgsm_location(car_wondex& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

