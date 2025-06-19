#ifndef obj_teltonikaFM_conditionH
#define obj_teltonikaFM_conditionH
#include "../device_data/teltonika/teltonikaFM_notify.h"
#include "../device_data/teltonika/teltonikaFM_commands.h"

namespace Teltonika
{

using namespace VisiPlug::Teltonika;

class car_teltonikaFM;

struct co_common_fm : public icar_condition<0>
{
  car_teltonikaFM& parent;
  co_common_fm(car_teltonikaFM& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};



}

#endif

