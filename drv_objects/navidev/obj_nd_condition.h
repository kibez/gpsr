#ifndef obj_nd_conditionH
#define obj_nd_conditionH

namespace Nd
{
class car_nd;


struct co_nd : public icar_condition<0>
{
  car_nd& parent;
  co_nd(car_nd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_mdt : public icar_condition<CONDITION_INTELLITRAC_MDT>
{
  car_nd& parent;
  co_mdt(car_nd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

