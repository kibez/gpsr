#ifndef obj_phantom_conditionH
#define obj_phantom_conditionH

#include <map>
#include "../device_data/phantom/phantom_notify.h"
#include "../device_data/phantom/phantom_commands.h"
#include "../easytrac/obj_easytrac_condition.h"

namespace Phantom
{
using namespace VisiPlug::Phantom;

class car_phantom;

struct co_phantom : public EasyTrac::co_easytrac
{
  co_phantom(EasyTrac::car_easytrac& parent);
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif
 