#ifndef obj_benefon_conditionH
#define obj_benefon_conditionH

#include <pkr_freezer.h>
#include <time.h>

class co_benefon_alert_attack : public icar_condition<CONDITION_ALERT_ATTACK>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_login : public icar_condition<CONDITION_LOGIN>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_logout : public icar_condition<CONDITION_LOGOUT>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_check_state_activate : public icar_condition<CONDITION_CHECK_STATE_ACTIVATE>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_check_alarm : public icar_condition<CONDITION_CHECK_ALARM>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_triger_alarm : public icar_condition<CONDITION_TRIGER_ALARAM>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_triger_activate : public icar_condition<CONDITION_TRIGER_ACTIVATE>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_request_waypoint : public icar_condition<CONDITION_REQUEST_WAYPOINT>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_create_route : public icar_condition<CONDITION_REQUEST_CREATE_ROUTE>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

class co_benefon_state : public icar_condition<CONDITION_BENEFON_UNKNOWN_STATE>
{
public:
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
  static bool do_parse(const std::vector<unsigned char>& ud,std::string& number,pkr_freezer& fr,time_t& datetime,bool as_short=true);
};

#endif

