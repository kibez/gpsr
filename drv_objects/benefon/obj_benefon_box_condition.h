#ifndef obj_benefon_box_conditionH
#define obj_benefon_box_conditionH

class co_benefon_box_state : public icar_condition<CONDITION_BENEFON_DINPUT>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

#endif

