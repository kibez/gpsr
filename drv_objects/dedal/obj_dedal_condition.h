#ifndef obj_dedal_conditionH
#define obj_dedal_conditionH
#include <vector>

class car_dedal;

struct co_dedal : public icar_condition<0>
{
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

#endif

