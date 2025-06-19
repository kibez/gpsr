#ifndef obj_starline_m15_conditionH
#define obj_starline_m15_conditionH

#include "../device_data/starline_m15/starline_m15_notify.h"

namespace StarLine_M15
{

using namespace VisiPlug::StarLine_M15;

struct co_sms : public icar_condition<CONDITION_STARLINE_M15_SMS>
{
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

} // namespace StarLine_M15


#endif
 