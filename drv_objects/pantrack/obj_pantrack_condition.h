#ifndef obj_pantrack_conditionH
#define obj_pantrack_conditionH
#include <vector>

class car_pantrack;

struct co_pantrack_sos : public icar_condition<CONDITION_ALERT_ATTACK>
{
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};


class co_pantrack_state : public icar_condition<CONDITION_ALERT_SIGNAL>
{
  typedef std::vector<unsigned> states_t;
private:
   states_t current_state;

   void check_states(const VisiPlug::Pantrack::state_t& val,states_t& active,states_t& inactive);
   states_t accept_states(const states_t& active,const states_t& inactive);
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};


#endif

