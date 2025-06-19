
#ifndef obj_easytrac_conditionH
#define obj_easytrac_conditionH

#include "../device_data/drozd/drozd_notify.h"

namespace Drozd
{

class car_drozd;
using namespace VisiPlug::Drozd;

struct co_track : public icar_condition<CONDITION_INTELLITRAC_TRACK>
{
  car_drozd& parent;
  co_track(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_fuel_change : public icar_condition<CONDITION_DROZD_FUEL_CHANGE>
{
  car_drozd& parent;
  co_fuel_change(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_fuel_counter : public icar_condition<CONDITION_DROZD_FUEL_COUNTER>
{
  car_drozd& parent;
  co_fuel_counter(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_thhmdt_info : public icar_condition<CONDITION_DROZD_THHMDT_INFO>
{
  car_drozd& parent;
  co_thhmdt_info(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_account_info : public icar_condition<CONDITION_DROZD_ACCOUNT_INFO>
{
  car_drozd& parent;
  co_account_info(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_mech_state : public icar_condition<CONDITION_DROZD_MECH_STATE>
{
  car_drozd& parent;
  co_mech_state(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_ths1_info : public icar_condition<CONDITION_DROZD_THS1_INFO>
{
  car_drozd& parent;
  co_ths1_info(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_trans_state : public icar_condition<CONDITION_DROZD_TRANS_STATE>
{
  car_drozd& parent;
  co_trans_state(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_engine_start : public icar_condition<CONDITION_DROZD_ENGINE_START>
{
  car_drozd& parent;
  co_engine_start(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_engine_stop : public icar_condition<CONDITION_DROZD_ENGINE_STOP>
{
  car_drozd& parent;
  co_engine_stop(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_fuel_level : public icar_condition<CONDITION_DROZD_FUEL_LEVEL>
{
  car_drozd& parent;
  co_fuel_level(car_drozd& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif
 