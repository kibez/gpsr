#ifndef obj_mobitel_conditionH
#define obj_mobitel_conditionH

class co_mobitel_guard_set : public icar_condition<CONDITION_GUARD_SET>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_mobitel_guard_reset : public icar_condition<CONDITION_GUARD_RESET>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_mobitel_reset_accept : public icar_condition<CONDITION_GUARD_RESET_ACCEPT>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_mobitel_alert_signal : public icar_condition<CONDITION_ALERT_SIGNAL>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_mobitel_alert_attack : public icar_condition<CONDITION_ALERT_ATTACK>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_mobitel_car_move : public icar_condition<CONDITION_ALERT_CAR_MOVE>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_mobitel_extern_power_on : public icar_condition<CONDITION_EXTERN_POWER_ON>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_mobitel_extern_power_off : public icar_condition<CONDITION_EXTERN_POWER_OFF>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

#endif

