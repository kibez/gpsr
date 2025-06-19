#ifndef obj_radom_conditionH
#define obj_radom_conditionH

class car_radom;

class co_radom_guard_set : public icar_condition<CONDITION_GUARD_SET>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_radom_guard_reset : public icar_condition<CONDITION_GUARD_RESET>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_radom_reset_accept : public icar_condition<CONDITION_GUARD_RESET_ACCEPT>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_radom_alert_signal : public icar_condition<CONDITION_ALERT_SIGNAL>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_radom_alert_attack : public icar_condition<CONDITION_ALERT_ATTACK>
{
  car_radom& parent;
  co_radom_alert_attack(car_radom& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_radom_car_move : public icar_condition<CONDITION_ALERT_CAR_MOVE>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_radom_outside_break : public icar_condition<CONDITION_OUTSIDE_BREAK>
{
  car_radom& parent;
  co_radom_outside_break(car_radom& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_radom_inside_break : public icar_condition<CONDITION_INSIDE_BREAK>
{
  car_radom& parent;
  co_radom_inside_break(car_radom& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_radom_main_power_lose : public icar_condition<CONDITION_RADOM_MAIN_POWER_LOSE>
{
  car_radom& parent;
  co_radom_main_power_lose(car_radom& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_radom_backup_power_lose : public icar_condition<CONDITION_RADOM_BACKUP_POWER_LOSE>
{
  car_radom& parent;
  co_radom_backup_power_lose(car_radom& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_radom_alert_sms : public icar_condition<CONDITION_RADOM_ALERT_SMS>
{
  car_radom& parent;
  co_radom_alert_sms(car_radom& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

#endif

