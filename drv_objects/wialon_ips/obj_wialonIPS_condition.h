#ifndef obj_wialon_IPS_conditionH 
#define obj_wialon_IPS_conditionH
#include "../../device_data/wialon_ips/wialonIPS_notify.h"
#include "../../device_data/wialon_ips/wialonIPS_commands.h"

namespace wialonIPS
{

using namespace VisiPlug::wialonIPS;

class /*car_teltonikaGH*/car_wialonIPS;

struct co_common : public icar_condition<0>
{
  car_wialonIPS& parent;
  co_common(car_wialonIPS& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_geo : public icar_condition<CONDITION_TELTONIKA_ZONE_IN>
{
  car_wialonIPS& parent;
  co_geo(car_wialonIPS& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};


}

#endif

