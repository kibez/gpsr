#ifndef obj_intellix1_conditionH
#define obj_intellix1_conditionH

namespace Intellix1
{
class car_intellix1;

struct co_intellix1 : public icar_condition<0>
{
  car_intellix1& parent;
  co_intellix1(car_intellix1& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_mdt : public icar_condition<CONDITION_INTELLITRAC_MDT>
{
  car_intellix1& parent;
  co_mdt(car_intellix1& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

