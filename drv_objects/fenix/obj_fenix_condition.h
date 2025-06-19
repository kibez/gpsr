#ifndef obj_fenix_conditionH
#define obj_fenix_conditionH
#include "../device_data/fenix/fenix_commands.h"

namespace Fenix
{
using namespace VisiPlug::Fenix;
class car_fenix;

struct co_common : public icar_condition<CONDITION_INTELLITRAC_TRACK>
{
  car_fenix& parent;
  co_common(car_fenix& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_gsm_location : public icar_condition<CONDITION_WONDEX_MULTI_GSM_LOCATION>
{
  car_fenix& parent;
  co_gsm_location(car_fenix& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

