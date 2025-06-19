#ifndef obj_teltonikaGH12_conditionH
#define obj_teltonikaGH12_conditionH
#include "../device_data/teltonika/teltonikaGH12_notify.h"
#include "../device_data/teltonika/teltonikaGH12_commands.h"

namespace Teltonika
{

using namespace VisiPlug::Teltonika;

class car_teltonikaGH;

struct co_common : public icar_condition<0>
{
  car_teltonikaGH& parent;
  co_common(car_teltonikaGH& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_geo : public icar_condition<CONDITION_TELTONIKA_ZONE_IN>
{
  car_teltonikaGH& parent;
  co_geo(car_teltonikaGH& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};


}

#endif

