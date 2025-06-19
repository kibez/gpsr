#ifndef obj_radom3_conditionH
#define obj_radom3_conditionH

class car_radom3;

struct co_radom3_input : public icar_condition<CONDITION_RADOM3_INPUT4>
{
  car_radom3& parent;
  co_radom3_input(car_radom3& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

#endif

